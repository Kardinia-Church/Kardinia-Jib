/**
    Main head handler
    Responsible for handling movement in the head


    TODO:
    Readd global speed adjustments
**/

#ifndef HEAD_HANDLER
#define HEAD_HANDLER

#include <Arduino.h>
#include <AccelStepper.h>
#include <EEPROMex.h>
#include "settings.h"


//The stepper object
class Stepper {
    private:
    AccelStepper _stepper;
    int _limitPin;
    long _maxPosition = 0;
    long _homePosition = 0;
    int _maxSpeed;
    int _defaultAcceleration;
    int _invertLimit;
    int _memoryStartAddress = -1;
    const int _totalMemoryAllocation = STEPPER_MEM_ALLOC;
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
    Stepper(AccelStepper stepper, int limitPin, boolean invert, int maxSpeed, int defaultAcceleration, int invertLimit=0) {
        _stepper = stepper;
        _limitPin = limitPin;
        _maxSpeed = maxSpeed;
        _defaultAcceleration = defaultAcceleration;
        _invertLimit = invertLimit;

        _stepper.setMaxSpeed(maxSpeed);
        _stepper.setAcceleration(defaultAcceleration);
        _stepper.setPinsInverted(invert, false, true);
        pinMode(limitPin, INPUT_PULLUP);
    }

    //Set where this steppers settings start in the EEPROM
    void setMemoryStartAddress(int addr) {
        _memoryStartAddress = addr;
    }

    //Set the current settings to memory
    boolean setSettingsToMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > END_OF_MEMORY) {
            Serial.println("[ERROR] Could not set stepper axis memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            Serial.print("[INFO] Set stepper axis to memory..");
            Serial.print("maxPosition@" + (String)_memoryStartAddress + " , ");
            EEPROM.writeLong(_memoryStartAddress, _maxPosition);
            Serial.print("homePosition@" + (String)(_memoryStartAddress + 4));
            EEPROM.writeLong(_memoryStartAddress + 4, _homePosition);
            Serial.println(" Done");
            return true;
        }
    }

    //Read the current settings from memory
    boolean readSettingsFromMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > END_OF_MEMORY) {
            Serial.println("[ERROR] Could not read stepper axis memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            //If all the values are set to 255 then set the defaults
            int amountOfUnsetData = 0;
            for(int i = _memoryStartAddress; i < _memoryStartAddress + _totalMemoryAllocation; i++) {if(EEPROM.read(i) == 255){amountOfUnsetData++;}}
            if(amountOfUnsetData == _totalMemoryAllocation) {
                //Data is not set default them
                Serial.println("[WARN] Stepper axis settings are not set. Setting defaults");
                setSettingsToMemory();
                return true;
            }
            else {
                _maxPosition = EEPROM.readLong(_memoryStartAddress);
                _homePosition = EEPROM.readLong(_memoryStartAddress + 4);
                return true;
            }
        }
    }

    //Returns true if the stepper is at the given limit
    boolean isAtLimit(LimitType type) {
        switch(type) {
            case LimitType::Min: {
                if(digitalRead(_limitPin) == _invertLimit) {
                    _stepper.move(1);
                    _stepper.setCurrentPosition(0);
                    return true;
                }
                break;
            }
            case LimitType::Max: {
                if(_stepper.currentPosition() >= _maxPosition) {
                    _stepper.move(-1);
                    _stepper.setCurrentPosition(_maxPosition);
                    return true;
                }
                break;
            }
            case LimitType::Any: {
               if(digitalRead(_limitPin) == _invertLimit) {
                    _stepper.move(1);
                    _stepper.setCurrentPosition(0);
                    return true;
                }
                else if(_stepper.currentPosition() >= _maxPosition) {
                    _stepper.move(-1);
                    _stepper.setCurrentPosition(_maxPosition);
                    return true;
                }
                break;
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
    long aimedStopPosition = 0;
    void move(float value, float globalSpeed=50.0, float acceleration=50.0) {
        //Make sure the globa speed and acceleration cannot go below 0 as this causes issues
        if(globalSpeed < 0){globalSpeed = 0;}
        if(acceleration < 0){acceleration = 0;}

        //Set the speed
        if(value != 0) {
            _stepper.setAcceleration(_defaultAcceleration * (acceleration/50.0));
            float speedDiv = abs(((value / 100.0) * 100) * globalSpeed/50.0);
           _stepper.setMaxSpeed(_maxSpeed * (speedDiv/100.0));
        }

        //Set the direction 
        if(value < -1.0) {
            _stepper.moveTo(-_maxPosition);
            aimedStopPosition = 0;
        }
        else if(value > 1.0){
            _stepper.moveTo(_maxPosition);
            aimedStopPosition = 0;
        }
        else {
            if(_stepper.targetPosition() != aimedStopPosition) {
                _stepper.stop();
                aimedStopPosition = _stepper.targetPosition();
            }
        }
    }

    //Calibrate the axis
    void calibate(int potPin) {
        Serial.println("[CAL] Calibration for axis begin.");
        Serial.println("[CAL] Please wait moving axis to min point to begin calibation");
        while(true) {
            //Move to min limit
            _maxPosition = 100000;
            _maxSpeed = 500;
            _defaultAcceleration = 1000000;
            _stepper.moveTo(-_maxPosition + 100);
            _stepper.setMaxSpeed(_maxSpeed / 2);
            _stepper.setAcceleration(_defaultAcceleration);
            _stepper.run();

            if(_stepper.distanceToGo() == 0) {
                //Failed to get to position
                Serial.println("[CAL] Failed to reach minimum position. Calibration aborted");
                return;
            }
            else if(isAtLimit(LimitType::Min)) {
                Serial.println("[CAL] At end limit");
                _stepper.stop();
                _stepper.setCurrentPosition(0);
                break;
            }
        }

        Serial.println("[CAL] Ready to start calibation!");

        //Calibate the speed pot first
        int potValue = analogRead(potPin);
        int minPotValue = 0;
        int maxPotValue = 1023;
        Serial.println("[CAL] The calibation will use the right most speed knob. We need to calibrate it first please set it to center and it will start calibrating in 5 seconds");
        delay(5000);
        Serial.println("[CAL] Move the pot to the 0%");
        while(true) {
            if(analogRead(potPin) >= potValue + 100 || analogRead(potPin) <= potValue - 100) {
                Serial.println("[CAL] Getting the value");
                delay(4000);
                minPotValue = analogRead(potPin);
                break;
            }
        }
        Serial.println("[CAL] Next set the speed knob to 100%");
        potValue = analogRead(potPin);
        while(true) {
            if(analogRead(potPin) >= potValue + 100 || analogRead(potPin) <= potValue - 100) {
                Serial.println("[CAL] Getting the value");
                delay(4000);
                maxPotValue = analogRead(potPin);
                break;
            }
        }

        Serial.println("[CAL] The axis will start moving at a speed set by the right speed knob. When the axis is at the desired position set the knob to 0% to set the value");
        while(true) {
            float speed = (float)analogRead(potPin) / (float)(maxPotValue - minPotValue);

            //If the pot is at 0% set the value
            if(speed < 0.1) {
                _stepper.stop();
                break;
            }
            else {
                //Move the axis at speed
                _stepper.moveTo(_maxPosition);
                _stepper.setMaxSpeed(_maxSpeed * speed);
                _stepper.run();
            }   
        }

        Serial.println("[CAL] The axis is now at the max position: " + (String)_stepper.currentPosition());
        _maxPosition = _stepper.currentPosition();
        _homePosition = _maxPosition / 2;
        Serial.println("[CAL] Storing values to EEPROM");
        setSettingsToMemory();
        Serial.println("[CAL] Done!");
    }

    //Home the stepper to the minumum value, will return true when at home position
    Stepper::HomeStatus home() {
        //Start the home proceedure if it hasn't started
        if(_stepper.targetPosition() != -_maxPosition + 100 && _stepper.targetPosition() != _homePosition) {
            _stepper.moveTo(-_maxPosition + 100);
        }

        if(_stepper.targetPosition() < 0) {
            //If we are currently moving toward the limit switch
            if(!isAtLimit(LimitType::Min)) {
                //Check if we have failed to home
                if(_stepper.distanceToGo() == 0){return HomeStatus::Failed;}

                _stepper.setMaxSpeed(_maxSpeed);
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

    //Check if the settings are valid.
    boolean checkSettings() {
        if((_homePosition == 0 && _maxPosition == 0) || (_homePosition > _maxPosition) || _homePosition < 0 || _maxPosition < 0) {return false;}
        return true;
    }

    //Print out the settings
    void printSettings() {
        Serial.print("[homePosition, maxPosition]=" + String(_homePosition) + "," + String(_maxPosition));
    }
};

class Head {
    public:    
    enum StepperAxis {
        X,
        Y
    };

    private:
    Stepper *_steppers[2];
    int _memoryStartAddr;
    public:
    //Constructor
    Head(Stepper &xStepper,  Stepper &yStepper, int memoryStartAddr) {
        _steppers[StepperAxis::X] = &xStepper;
        _steppers[StepperAxis::Y] = &yStepper;
        _memoryStartAddr = memoryStartAddr;
        _steppers[StepperAxis::X]->setMemoryStartAddress(_memoryStartAddr);
        _steppers[StepperAxis::Y]->setMemoryStartAddress(_memoryStartAddr + STEPPER_MEM_ALLOC);
    }

    //Read the settings from memory
    boolean readSettingsFromMemory() {
        if(_steppers[StepperAxis::X]->readSettingsFromMemory() && _steppers[StepperAxis::Y]->readSettingsFromMemory()) {
            return true;
        }
        return false;
    }

    //Check if the settings are valid
    boolean checkSettings() {
        if(!_steppers[StepperAxis::X]->checkSettings() || !_steppers[StepperAxis::Y]->checkSettings()) {
            return false;
        }
        return true;
    }

    //Print out the settings
    void printSettings() {
        Serial.print("X");
        _steppers[StepperAxis::X]->printSettings();
        Serial.print("\nY");
        _steppers[StepperAxis::Y]->printSettings();
    }

    //Calibrate the axis'
    void calibrate(int potPin) {
        while(Serial.available() == 1){
            digitalWrite(DEBUG_LED, 1);
            delay(100);
            digitalWrite(DEBUG_LED, 0);
            delay(100);
        }

        Serial.println("[CAL] Starting calibration of both X and Y stepper axis");
        _steppers[StepperAxis::X]->calibate(potPin);
        _steppers[StepperAxis::Y]->calibate(potPin);
        Serial.println("[CAL] Completed calibration. Rehoming");
        home();
    }

    //Home the head
    Stepper::HomeStatus home() {
        Serial.print("[INFO] - Attempting to home");
        int homeDot = 0;
        while(true) {
            homeDot++;
            if(homeDot % 5000 == 0){Serial.print(".");}

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
    void move(StepperAxis axis, float speed, float globalSpeed=50.0, float acceleration=50.0) {
        _steppers[axis]->move(speed, globalSpeed, acceleration);
    }

    //Move X and Y axis with speed. Speed divsor sets the global speed higher a % faster the speed. If no acceleration is defined it will default
    void moveXY(float speedX, float speedY, float globalSpeed=50.0, float acceleration=50.0) {
        _steppers[StepperAxis::X]->move(speedX, globalSpeed, acceleration);
        _steppers[StepperAxis::Y]->move(speedY, globalSpeed, acceleration);
    }

    //The main loop. Returns true if a stepper is moving
    boolean run() {
        boolean isRunning = false;
        if(_steppers[StepperAxis::X]->run() == Stepper::Status::Moving){isRunning=true;}
        if(_steppers[StepperAxis::Y]->run() == Stepper::Status::Moving){isRunning=true;}
    }
};

#endif