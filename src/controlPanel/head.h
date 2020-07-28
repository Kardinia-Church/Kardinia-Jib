/**
    Main head handler
    Responsible for handling movement in the head
**/

#ifndef HEAD_HANDLER
#define HEAD_HANDLER

#include <Arduino.h>
#include "AccelStepper/src/AccelStepper.h"
#include <EEPROMex.h>
#include "settings.h"

#define STEP_SAFE_ZONE 20

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
    bool _successfullyReset = false;
    bool _movingToPosition = false;
    bool _movingRelative = false;
    bool _isStopping = false;
    // const int _totalMemoryAllocation = STEPPER_MEM_ALLOC;
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
    Stepper(AccelStepper stepper, int limitPin, boolean invert, int maxSpeed, int defaultAcceleration, long maxPosition, int invertLimit=0) {
        _stepper = stepper;
        _limitPin = limitPin;
        _maxSpeed = maxSpeed;
        _maxPosition = maxPosition;
        _homePosition = maxPosition / 2;
        _defaultAcceleration = defaultAcceleration;
        _invertLimit = invertLimit;

        _stepper.setMaxSpeed(maxSpeed);
        _stepper.setAcceleration(defaultAcceleration);
        _stepper.setPinsInverted(invert, false, true);
        pinMode(limitPin, INPUT_PULLUP);
    }

    //Returns true if the stepper is at the given limit
    boolean isAtLimit(LimitType type) {
        switch(type) {
            case LimitType::Min: {
                if(digitalRead(_limitPin) == _invertLimit) {
                    while(digitalRead(_limitPin) == _invertLimit) {
                        _stepper.setSpeed(_maxSpeed);
                        _stepper.setAcceleration(_defaultAcceleration);
                        _stepper.move(1);
                        _stepper.run();
                    }
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
                    while(digitalRead(_limitPin) == _invertLimit) {
                        _stepper.setSpeed(_maxSpeed);
                        _stepper.setAcceleration(_defaultAcceleration);
                        _stepper.move(1);
                        _stepper.run();
                    }
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

    boolean isMovingToPosition() {
        return _movingToPosition;
    }

    bool isMovingRelative() {
        return _movingRelative;
    }

    //The main run with end limit handling
    Status run() {
        if(!_successfullyReset){return Status::Stopped;}
        if(isAtLimit(LimitType::Min)) {
            _stepper.setCurrentPosition(STEP_SAFE_ZONE);
            if(_stepper.targetPosition() != _stepper.currentPosition()){_stepper.run();}
            return Status::AtEndLimit;
        }
        else if(isAtLimit(LimitType::Max)) {
            _stepper.setCurrentPosition(_maxPosition - STEP_SAFE_ZONE);
            if(_stepper.targetPosition() != _stepper.currentPosition()){_stepper.run();}
            return Status::AtEndLimit;   
        }
        else {
            if(_stepper.currentPosition() < STEP_SAFE_ZONE){_stepper.moveTo(STEP_SAFE_ZONE);}
            if(_stepper.currentPosition() > _maxPosition - STEP_SAFE_ZONE){_stepper.moveTo(_maxPosition - STEP_SAFE_ZONE);}

            _stepper.run();
            if(_stepper.isRunning()){return Status::Moving;}
            else {_movingToPosition = false; _movingRelative = false; _isStopping = false; return Status::Stopped;}
        }
    }

    //Move the stepper in a given direction at speed. Value is a float between -100.0% -> 100.0%. Acceleration is the acceleration again in percentage
    long aimedStopPosition = 0;
    float previousAcceleration = 0;
    void move(float speed, float acceleration=100.0) {
        if(speed < 0 && _stepper.distanceToGo() >= 0) {
            _stepper.moveTo(STEP_SAFE_ZONE);
            _stepper.setAcceleration(10000);
            _isStopping = false;
        }
        else if(speed > 0 && _stepper.distanceToGo() <= 0) {
            _stepper.moveTo(_maxPosition - STEP_SAFE_ZONE);
            _stepper.setAcceleration(10000);
            _isStopping = false;
        }
        else if(speed == 0 && !_isStopping) {
            _stepper.stop();
            _isStopping = true;
        }
        if(!_isStopping) {
            _stepper.setMaxSpeed(abs((float)_maxSpeed * (speed / 100.0)));
        }
    }

    //Move to a specific location
    void moveTo(long position, float speed = 100.0, float acceleration = 100.0) {
        _stepper.setMaxSpeed(_maxSpeed * (speed/100.0));
        _stepper.setAcceleration(_defaultAcceleration * (acceleration/100.0));
        if(position > _maxPosition - STEP_SAFE_ZONE){position = _maxPosition - STEP_SAFE_ZONE;}
        _stepper.moveTo(position);
        _movingToPosition = true;
    }

    //Move relative to the current position
    void moveRelative(long distance, float speed = 100.0, float acceleration = 100.0) {
        _stepper.setMaxSpeed(_maxSpeed * (speed/100.0));
        _stepper.setAcceleration(_defaultAcceleration * (acceleration/100.0));
        _stepper.move(distance);
        _movingRelative = true;
    }

    void goHome(float speed = 100.0, float acceleration = 100.0) {
        moveTo(_homePosition, speed, acceleration);
    }

    //Stop
    void stop(float acceleration = 100.0){
        _stepper.setAcceleration(_defaultAcceleration * (acceleration/100.0));
        _stepper.stop();
    }

    //Reset the stepper to the minumum value, will return true when at home position
    Stepper::HomeStatus reset() {
        //Start the home proceedure if it hasn't started
        if(_stepper.targetPosition() != -100 && _stepper.targetPosition() != _homePosition) {
            _stepper.setCurrentPosition(_maxPosition);
            _stepper.moveTo(-100);
        }

        if(_stepper.targetPosition() == -100) {
            //If we are currently moving toward the limit switch
            if(!isAtLimit(LimitType::Min)) {
                //Check if we have failed to home
                if(_stepper.distanceToGo() == 0){return HomeStatus::Failed;}

                _stepper.setMaxSpeed(_maxSpeed);
                _stepper.setAcceleration(1000);
                _stepper.run();
                return HomeStatus::MovingToMin;
            }
            else {
                //Completed min head to home position
                _stepper.setCurrentPosition(0);
                _stepper.moveTo(STEP_SAFE_ZONE);
                _stepper.runToPosition();
                _stepper.setCurrentPosition(0);
                _stepper.setMaxSpeed(_maxSpeed);
                _stepper.setAcceleration(1000);
                _stepper.moveTo(_homePosition);
                _stepper.run();
                _successfullyReset = true;
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

    //Is the stepper moving?
    boolean isMoving() {
        return _successfullyReset && (_stepper.distanceToGo() != 0);
    }

    void setMaxSpeed(float speed) {
        _stepper.setMaxSpeed((float)_maxSpeed * (speed/100.0));
    }

    long maxPosition() {
        return _maxPosition;
    }
};

class Head {
    public:    
    enum StepperAxis {
        X,
        Y
    };
    Stepper *_steppers[2];

    private:
    // int _memoryStartAddr;
    public:
    //Constructor
    Head(Stepper &xStepper,  Stepper &yStepper) {
        _steppers[StepperAxis::X] = &xStepper;
        _steppers[StepperAxis::Y] = &yStepper;
    }

    //Home the head
    Stepper::HomeStatus reset() {
        Serial.print("Attempting to reset");
        int homeDot = 0;
        while(true) {
            homeDot++;
            if(homeDot % 5000 == 0){Serial.print(".");}

            //If both steppers are complete we homed successfully!
            if(_steppers[StepperAxis::X]->reset() == Stepper::HomeStatus::Complete && _steppers[StepperAxis::Y]->reset() == Stepper::HomeStatus::Complete) {
                Serial.println(" Done!");
                return Stepper::HomeStatus::Complete;
            }
            //If one of the steppers has failed and the other one has failed/completed handle it
            else if(_steppers[StepperAxis::X]->reset() == Stepper::HomeStatus::Failed || _steppers[StepperAxis::Y]->reset() == Stepper::HomeStatus::Failed) {
                if(_steppers[StepperAxis::X]->reset() >= Stepper::HomeStatus::Complete && _steppers[StepperAxis::Y]->reset() >= Stepper::HomeStatus::Complete) {
                    Serial.print(" ERROR! -");
                    if(_steppers[StepperAxis::X]->reset() == Stepper::HomeStatus::Failed){Serial.print(" Stepper X has failed to reset");}
                    if(_steppers[StepperAxis::Y]->reset() == Stepper::HomeStatus::Failed){Serial.print(" Stepper Y has failed to reset");}
                    Serial.println();
                    return Stepper::HomeStatus::Failed;
                }
            }
        }
    }

    //Is one of the axis' moving?
    bool isMoving() {
        return _steppers[StepperAxis::X]->isMoving() || _steppers[StepperAxis::Y]->isMoving();
    }

    bool isMovingRelative() {
        return _steppers[StepperAxis::X]->isMovingRelative() || _steppers[StepperAxis::Y]->isMovingRelative();
    }

    //Move a axis with speed and acceleration. Speed is between -100% - 100% (backward, forward) and acceleration is 0% - 100%
    void move(StepperAxis axis, float speed, float globalSpeed=100.0, float acceleration=100.0) {
        _steppers[axis]->move(speed, acceleration);
    }

    void setMaxSpeed(float speed) {
        _steppers[StepperAxis::X]->setMaxSpeed(speed);
        _steppers[StepperAxis::Y]->setMaxSpeed(speed);
    }

    //Move X and Y axis with speed. Speed divsor sets the global speed higher a % faster the speed. If no acceleration is defined it will default
    void moveXY(float speedX, float speedY, float acceleration=100.0) {
        _steppers[StepperAxis::X]->move(speedX, acceleration);
        _steppers[StepperAxis::Y]->move(speedY, acceleration);
    }

    //Move relative to the current position
    void moveRelative(long x, long y, float speed = 100.0, float acceleration = 100.0) {
        _steppers[StepperAxis::X]->moveRelative(x, speed, acceleration);
        _steppers[StepperAxis::Y]->moveRelative(y, speed, acceleration);
    }

    void goHome(float globalSpeed=100.0, float acceleration=100.0) {
        _steppers[StepperAxis::X]->goHome(globalSpeed, acceleration);
        _steppers[StepperAxis::Y]->goHome(globalSpeed, acceleration);
    }

    boolean movingToPosition() {
        return _steppers[StepperAxis::X]->isMovingToPosition() || _steppers[StepperAxis::Y]->isMovingToPosition();
    }

    void stop(float acceleration=100.0) {
        _steppers[StepperAxis::X]->stop(acceleration);
        _steppers[StepperAxis::Y]->stop(acceleration);
    }

    //Move to a specific location
    void moveToXY(long x, long y, float speed = 100.0, float accleration = 100.0) {
        _steppers[StepperAxis::X]->moveTo(x, speed, accleration);
        _steppers[StepperAxis::Y]->moveTo(y, speed, accleration);
    }

    //The main loop. Returns true if a stepper is moving
    boolean run() {
        boolean isRunning = false;
        if(_steppers[StepperAxis::X]->run() == Stepper::Status::Moving){isRunning=true;}
        if(_steppers[StepperAxis::Y]->run() == Stepper::Status::Moving){isRunning=true;}
        return isRunning;
    }
};

#endif