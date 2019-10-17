/**
    Main head handler
    Responsible for handling movement in the head
**/

#ifndef HEAD_HANDLER
#define HEAD_HANDLER

#include <Arduino.h>
#include <AccelStepper.h>
#include "settings.h"

//The stepper object
class Stepper {
    private:
    AccelStepper _stepper;
    int _limitPin;
    int _maxPosition;
    int _homePosition;
    int _maxSpeed;
    int _defaultAcceleration;
    int _invertLimit;
    int _memoryStartAddress = -1;
    const int _totalMemoryAllocation = 2;
    public:
    enum LimitType {
        Min,
        Max,
        Any
    };
    enum HomeStatus {
        MovingToMin,
        MovingToHome,
        Complete,
        Failed
    };
    enum Status {
        Moving,
        Stopped,
        AtEndLimit
    };
    Stepper(AccelStepper stepper, int limitPin, boolean invert, int maxPosition, int homePosition, int maxSpeed, int defaultAcceleration, int invertLimit=0) {
        _stepper = stepper;
        _limitPin = limitPin;
        _maxPosition = maxPosition;
        _homePosition = homePosition;
        _maxSpeed = maxSpeed;
        _defaultAcceleration = defaultAcceleration;

        _stepper.setMaxSpeed(maxSpeed);
        _stepper.setAcceleration(defaultAcceleration);
        _stepper.setPinsInverted(invert, false, true);
        pinMode(limitPin, INPUT);
    }

    //Set where this steppers settings start in the EEPROM
    void setMemoryStartAddress(int addr) {
        _memoryStartAddress = addr;
    }

    //Set the current settings to memory
    boolean setSettingsToMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > EEPROM.length()) {
            Serial.println("[ERROR] Could not read axis memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            int currentAddress = _memoryStartAddress;
            Serial.print("[INFO] Set axis settings to memory... ");
            Serial.print("maxPosition@" + (String)currentAddress + " , ");
            EEPROM.update(currentAddress++, _maxPosition);
            Serial.print("homePosition@" + (String)currentAddress + " , ");
            EEPROM.update(currentAddress++, _homePosition);
            Serial.println(" Done");
            return true;
        }
    }

    //Read the current settings from memory
    boolean readSettingsFromMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > EEPROM.length()) {
            Serial.println("[ERROR] Could not read axis memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            //If all the values are set to 255 then set the defaults
            int amountOfUnsetData = 0;
            for(int i = _memoryStartAddress; i < _memoryStartAddress + _totalMemoryAllocation; i++) {if(EEPROM.read(i) == 255){amountOfUnsetData++;}}
            if(amountOfUnsetData >= _totalMemoryAllocation / 2) {
                //Data is not set default them
                Serial.println("[WARN] Axis settings are not set. Setting defaults");
                setSettingsToMemory();
            }
            else {
                //Read the data from memory       
                int currentAddress = _memoryStartAddress;
                Serial.print("[INFO] Reading axis settings from memory... ");
                Serial.print("maxPosition@" + (String)currentAddress + " , ");
                _maxPosition = EEPROM.read(currentAddress++);
                Serial.print("homePosition@" + (String)currentAddress + " , ");
                _homePosition = EEPROM.read(currentAddress++);
                Serial.println(" Done");
            }
            return true;
        }
    }

    //Returns true if the stepper is at the given limit
    boolean isAtLimit(LimitType type) {
        switch(type) {
            case LimitType::Min: {
                if(digitalRead(_limitPin) == _invertLimit) {
                    return true;
                }
                break;
            }
            case LimitType::Max: {
                if(_stepper.currentPosition() >= _maxPosition) {
                    return true;
                }
                break;
            }
            case LimitType::Any: {
                if(digitalRead(_limitPin) == _invertLimit || _stepper.currentPosition() >= _maxPosition) {
                    return true;
                }
            }
        }

        return false;
    }

    //The main run with end limit handling
    Status run() {
        if(isAtLimit(LimitType::Min)) {
            _stepper.stop();
            _stepper.moveTo(1);
            while(_stepper.distanceToGo() != 0){_stepper.run();}
            return Status::AtEndLimit;
        }
        else if(isAtLimit(LimitType::Max)) {
            _stepper.stop();
            _stepper.moveTo(_maxPosition - 1);
            while(_stepper.distanceToGo() != 0){_stepper.run();}
            return Status::AtEndLimit;   
        }
        else {
            _stepper.run();
            if(_stepper.distanceToGo() != 0){return Status::Moving;}
            else {return Status::Stopped;}
        }
    }

    //Move the stepper in a given direction at speed. Value is a float between -100.0% -> 100.0%. Acceleration is the acceleration again in percentage
    void move(float value, float acceleration=-1) {
        if(acceleration == -1){acceleration = 100;}
        //Set the speed
        if(value != 0) {
            if(!(_stepper.distanceToGo() != 0 && abs(value) < abs((_stepper.speed() / _maxSpeed) * 100))) {
                _stepper.setMaxSpeed(abs(_maxSpeed * value/100.0));
            }
            
            _stepper.setAcceleration(_defaultAcceleration * acceleration/100.0);
        }

        //Set the direction 
        if(value < 0) {
            _stepper.move(-_maxPosition);
        }
        else if(value > 0){
            _stepper.move(_maxPosition);
        }
        else {
            if(_stepper.distanceToGo() != 0) {
                _stepper.setMaxSpeed(_stepper.speed());
                _stepper.stop();
            }
        }
    }

    //Calibrate the axis
    void calibate(int potPin) {
        Serial.println("[CAL] Calibration for axis begin.");
        Serial.println("[CAL] Please wait moving axis to min point to begin calibation");
        while(true) {
            if(_stepper.distanceToGo() == 0) {
                //Failed to get to position
                Serial.println("[CAL] Failed to reach minimum position. Calibration aborted");
                return;
            }
            else if(isAtLimit(LimitType::Min)) {
                _stepper.stop();
                _stepper.setCurrentPosition(0);
                break;
            }

            //Move to min limit
            _stepper.moveTo(-_maxPosition * 2);
            _stepper.setMaxSpeed(_maxSpeed / 2);
            _stepper.setAcceleration(_defaultAcceleration);
            _stepper.run();
        }

        Serial.println("[CAL] Ready to start calibation!");

        //Calibate the speed pot first
        int potValue = analogRead(potPin);
        int minPotValue = 0;
        int maxPotValue = 1023;
        Serial.println("[CAL] The calibation will use the right most speed knob. We need to calibrate it first please set it to 0%");
        while(true) {
            if(analogRead(potPin) >= potValue + 100 || analogRead(potPin) <= potValue - 100) {
                Serial.println("[CAL] Getting the value");
                delay(1000);
                minPotValue = analogRead(potPin);
                break;
            }
        }
        Serial.println("[CAL] Next set the speed knob to 100%");
        potValue = analogRead(potPin);
        while(true) {
            if(analogRead(potPin) >= potValue + 100 || analogRead(potPin) <= potValue - 100) {
                Serial.println("[CAL] Getting the value");
                delay(1000);
                maxPotValue = analogRead(potPin);
                break;
            }
        }

        Serial.println("[CAL] The axis will start moving at a speed set by the right speed knob. When the axis is at the desired position set the knob to 0% to set the value");
        while(true) {
            float speed = analogRead(potPin) / (maxPotValue - minPotValue) * 100.0;

            //If the pot is at 0% set the value
            if(speed < 1.0) {
                _stepper.stop();
                break;
            }
            else {
                //Move the axis at speed
                move(speed);
                _stepper.run();
            }   
        }

        Serial.println("[CAL] The axis is now at the max position: " + _stepper.currentPosition());
        _maxPosition = _stepper.currentPosition();
        Serial.println("[CAL] Storing values to EEPROM");
        setSettingsToMemory();
        Serial.println("[CAL] Done!");
    }

    //Home the stepper to the minumum value, will return true when at home position
    Stepper::HomeStatus home() {
        //Start the home proceedure if it hasn't started
        if(_stepper.targetPosition() != -_maxPosition * 2 && _stepper.targetPosition() != _homePosition) {
            _stepper.moveTo(-_maxPosition * 2);
        }

        if(_stepper.targetPosition() < 0) {
            //If we are currently moving toward the limit switch
            if(!isAtLimit(LimitType::Min)) {
                //Check if we have failed to home
                if(_stepper.distanceToGo() == 0){return HomeStatus::Failed;}

                _stepper.setMaxSpeed(_maxSpeed / 2);
                _stepper.setAcceleration(_defaultAcceleration);
                _stepper.run();
                return HomeStatus::MovingToMin;
            }
            else {
                //Completed min head to home position
                _stepper.setCurrentPosition(0);
                _stepper.setMaxSpeed(_maxSpeed);
                _stepper.setAcceleration(_defaultAcceleration);
                _stepper.moveTo(_homePosition);
                _stepper.run();
                return HomeStatus::MovingToHome;
            }
        }
        else if(_stepper.currentPosition() != _homePosition) {
            //If we are currently heading toward home
            _stepper.run();
            return HomeStatus::MovingToHome;
        }
        else {
            //Home has Completed
            return HomeStatus::Complete;
        }
    }
};

class Head {
    private:
    Stepper *_steppers[2];
    int _memoryStartAddr;
    enum StepperAxis {
        X,
        Y
    };

    public:
    Head(Stepper xStepper,  Stepper yStepper, int memoryStartAddr) {
        _steppers[StepperAxis::X] = &xStepper;
        _steppers[StepperAxis::Y] = &yStepper;
        _memoryStartAddr = memoryStartAddr;
    }

    //Home the head
    Stepper::HomeStatus home() {
        Serial.print("[INFO] - Attempting to home");
        while(true) {
            Serial.print(".");

            //If both steppers are complete we homed successfully!
            if(_steppers[StepperAxis::X]->home() == Stepper::HomeStatus::Complete && _steppers[StepperAxis::Y]->home() == Stepper::HomeStatus::Complete) {
                Serial.println(" Done!");
                return Stepper::HomeStatus::Complete;
            }
            //If one of the steppers has failed and the other one has failed/completed handle it
            else if(_steppers[StepperAxis::X]->home() == Stepper::HomeStatus::Failed || _steppers[StepperAxis::Y]->home() == Stepper::HomeStatus::Failed) {
                if(_steppers[StepperAxis::X]->home() >= Stepper::HomeStatus::Complete && _steppers[StepperAxis::Y]->home() >= Stepper::HomeStatus::Complete) {
                    Serial.println(" ERROR!");
                    if(_steppers[StepperAxis::X]->home() == Stepper::HomeStatus::Failed){Serial.println("[ERROR] Stepper X has failed to home");}
                    if(_steppers[StepperAxis::Y]->home() == Stepper::HomeStatus::Failed){Serial.println("[ERROR] Stepper Y has failed to home");}
                    return Stepper::HomeStatus::Failed;
                }
            }
        }
    }

    //Move a axis with speed and acceleration. Speed is between -100% - 100% (backward, forward) and acceleration is 0% - 100%
    void move(StepperAxis axis, float speed, float acceleration) {
        _steppers[axis]->move(speed, acceleration);
    }

    //Move X and Y axis with speed. If no acceleration is defined it will default
    void moveXY(float speedX, float speedY, float accelerationX=-1, float accelerationY=-1) {
        _steppers[StepperAxis::X]->move(speedX, accelerationX);
        _steppers[StepperAxis::Y]->move(speedY, accelerationY);
    }

    //The main loop. Returns true if a stepper is moving
    boolean run() {
        boolean isRunning = false;
        if(_steppers[StepperAxis::X]->run() == Stepper::Status::Moving){isRunning=true;}
        if(_steppers[StepperAxis::Y]->run() == Stepper::Status::Moving){isRunning=true;}
    }
};

#endif