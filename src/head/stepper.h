/*
Stepper class responsible for processing the steppers actions
By Kardinia Church 2020
*/

#include <Arduino.h>
#include "SpeedStepper/SpeedStepper.h"

#define STEPPER_MEM_ALLOC 8

class Stepper {
    public:
    enum Status {
        FindingEndStop,
        Stopped,
        AtEndLimit,
        Moving
    };
    Stepper(int stepPin, int dirPin, int limitPin, bool invert, float maxSpeed, float defaultAcceleration, bool invertLimit = false) {
        _stepper = new SpeedStepper(stepPin, dirPin);
        _limitPin = limitPin;
        _invert = invert;
        _maxSpeed = maxSpeed;
        _defaultAcceleration = defaultAcceleration;
        _invertLimit = invertLimit;
    }
    //Goto the home position
    void gotoHomePosition() {
        
    }
    bool setup(int memoryStartAddr) {
        _memoryStartAddress = memoryStartAddr;
        pinMode(_limitPin, INPUT_PULLUP);
        if(!readSettingsFromMemory()){return false;}
        if(_invert){_stepper->invertDirectionLogic();}
        _stepper->setPlusLimit(_maxPosition);
        _stepper->setMinusLimit(0);
        _stepper->setMaxSpeed(_maxSpeed);
        _stepper->setMinSpeed(1);
        _stepper->setAcceleration(_defaultAcceleration);
    }

    //Goto a set position
    bool gotoPosition(long position, float speed = -1, float acceleration = -1) {
        if(speed != -1){_stepper->setSpeed(speed);}
        if(acceleration != -1){_stepper->setAcceleration(acceleration);}

        //Calculate steps required to accelerate
        int32_t stepsAcceleration = pow(_stepper->getSpeed(), 2) / 2 * acceleration;

        _currentAimedPosition = position;


    }

    //Move toward 0 to find the 0 point using the limit switch will return true when found
    bool findMinusLimit() {
        if(_currentState != Status::FindingEndStop) {
            _stepper->setMinusLimit(-_maxPosition);
            _stepper->setMaxSpeed(_maxSpeed / 4);
            _stepper->setAcceleration(_defaultAcceleration);
        }
        _stepper->stepReverse();

        if(endStopTriggered()) {
            //We have found the endstop
            _stepper->hardStop();
            _stepper->setMinusLimit(0);
            _stepper->setCurrentPosition(0);
            _stepper->setMaxSpeed(_maxSpeed);
            return true;
        }
    }
    //Get the current status of the stepper
    Status getStatus() {return _currentState;}

    private:
    SpeedStepper *_stepper;
    long _maxPosition;
    long _homePosition;
    int _memoryStartAddress = -1;
    int _limitPin = -1;
    bool _invert = false;
    float _maxSpeed = 0;
    float _defaultAcceleration = 0;
    bool _invertLimit = false;
    Status _currentState = Status::Stopped;
    long _currentAimedPosition = 0;

    //Attempt to read settings from memory.
    bool readSettingsFromMemory() {
        _maxPosition = EEPROM.readLong(_memoryStartAddress);
        _homePosition = EEPROM.readLong(_memoryStartAddress + 4);
        return true;
    }

    //Attempt to save the settings to memory
    bool saveSettingsToMemory() {
        EEPROM.writeLong(_memoryStartAddress, _maxPosition);
        EEPROM.writeLong(_memoryStartAddress + 4, _homePosition);
        return true;
    }

    //Is the endstop triggered?
    bool endStopTriggered() {return digitalRead(_limitPin);}

    //Returns the steps required to slow down at the current speed
    int32_t stepsRequiredToSlowDown() {
        return pow(_stepper->getSpeed(), 2) / 2 * acceleration;
    }
};