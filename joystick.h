/**
    Main Joystick Handler
    Responsible for handling the joystick

    There is something hardware wise wrong with the z axis
**/

#ifndef JOYSTICK_HANDLER
#define JOYSTICK_HANDLER

#include <Arduino.h>
#include <EEPROM.h>
#include "settings.h"

class JoyStick {
    private:
    enum ValueType {
        Pin,
        Center,
        MinValue,
        MaxValue
    };
    enum Axis {
        X,
        Y,
        Z
    };
    int _axisSettings[3][4];
    int _memoryStartAddress;
    const int _deadzone = 10;

    public:
    //Get a axis position as a raw value
    int getValue(Axis axis) {
        int val = _axisSettings[axis][ValueType::Center] - analogRead(_axisSettings[axis][ValueType::Pin]);
        if(val > _deadzone) {
            val = val - _deadzone;
        }
        else if(val < -_deadzone) {
            val = val + _deadzone;
        }
        else {val = 0;}
        return val;
    }

    //Get the percentage of a axis
    float getPercentage(Axis axis) {
        float divider = _axisSettings[axis][ValueType::MaxValue] - _axisSettings[axis][ValueType::MinValue];
        float val = ((float)getValue(axis)/divider) * 100.0;
        if(val < -100.0){val = -100.0;}
        if(val > 100.0) {val = 100.0;}
        return val;
    }

    //Output the joystick values to console
    void joystickDebug() {
        int value = 0;
        float percentage = 0.0;
        Serial.print("[DEBUG][JOY] - Raw XYZ:");
        Serial.print(analogRead(_axisSettings[Axis::X][ValueType::Pin]));
        Serial.print(", ");
        Serial.print(analogRead(_axisSettings[Axis::Y][ValueType::Pin]));
        Serial.print(", ");
        Serial.print(analogRead(_axisSettings[Axis::Z][ValueType::Pin]));
        Serial.print("; VAL XYZ:");
        value = getValue(Axis::X);
        percentage = getPercentage(Axis::X);
        Serial.print((String)percentage + "(" + value + "), ");
        
        value = getValue(Axis::Y);
        percentage = getPercentage(Axis::Y);
        Serial.print((String)percentage + "(" + value + "), ");

        value = getValue(Axis::Z);
        percentage = getPercentage(Axis::Z);
        Serial.print((String)percentage + "(" + value + ")");

        Serial.println(";");
    }

    //Set the axis values
    void setAxisValues(Axis axis, int pin, int center, int minValue, int maxValue) {
        _axisSettings[axis][ValueType::Pin] = pin;
        pinMode(pin, INPUT);
        _axisSettings[axis][ValueType::Center] = center;
        _axisSettings[axis][ValueType::MinValue] = minValue;
        _axisSettings[axis][ValueType::MaxValue] = maxValue;
    }

    //Read the settings from memory XYZ[min, max, center]
    boolean readSettingsFromMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + 9 > EEPROM.length()) {
            Serial.println("[ERROR] Could not read joystick memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            //If all the values are set to 255 then set the defaults
            int amountOfUnsetData = 0;
            for(int i = _memoryStartAddress; i < _memoryStartAddress + 9; i++) {if(EEPROM.read(i) == 255){amountOfUnsetData++;}}
            if(amountOfUnsetData == 9) {
                //Data is not set default them
                Serial.println("[WARN] Joystick settings are not set. Setting defaults");
                setSettingsToMemory();
            }
            else {
                //Read the data from memory       
                int currentAddress = _memoryStartAddress;
                Serial.print("[INFO] Reading joystick settings from memory... ");
                Serial.print("X@" + (String)currentAddress + " , ");
                _axisSettings[Axis::X][ValueType::MinValue] = EEPROM.read(currentAddress++);
                _axisSettings[Axis::X][ValueType::MaxValue] = EEPROM.read(currentAddress++);
                _axisSettings[Axis::X][ValueType::Center] = EEPROM.read(currentAddress++);
                Serial.print("Y@" + (String)currentAddress + " , ");
                _axisSettings[Axis::Y][ValueType::MinValue] = EEPROM.read(currentAddress++);
                _axisSettings[Axis::Y][ValueType::MaxValue] = EEPROM.read(currentAddress++);
                _axisSettings[Axis::Y][ValueType::Center] = EEPROM.read(currentAddress++);
                Serial.print("Z@" + (String)currentAddress + " , ");
                _axisSettings[Axis::Z][ValueType::MinValue] = EEPROM.read(currentAddress++);
                _axisSettings[Axis::Z][ValueType::MaxValue] = EEPROM.read(currentAddress++);
                _axisSettings[Axis::Z][ValueType::Center] = EEPROM.read(currentAddress++);
                Serial.println(" Done");
            }
            return true;
        }
    }

    //Read the settings from memory XYZ[min, max, center]
    boolean setSettingsToMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + 9 > EEPROM.length()) {
            Serial.println("[ERROR] Could not read joystick memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            int currentAddress = _memoryStartAddress;
            Serial.print("[INFO] Set joystick settings to memory... ");
            Serial.print("X@" + (String)currentAddress + " , ");
            EEPROM.update(currentAddress++, _axisSettings[Axis::X][ValueType::MinValue]);
            EEPROM.update(currentAddress++, _axisSettings[Axis::X][ValueType::MaxValue]);
            EEPROM.update(currentAddress++, _axisSettings[Axis::X][ValueType::Center]);
            Serial.print("Y@" + (String)currentAddress + " , ");
            EEPROM.update(currentAddress++, _axisSettings[Axis::Y][ValueType::MinValue]);
            EEPROM.update(currentAddress++, _axisSettings[Axis::Y][ValueType::MaxValue]);
            EEPROM.update(currentAddress++, _axisSettings[Axis::Y][ValueType::Center]);
            Serial.print("Z@" + (String)currentAddress + " , ");
            EEPROM.update(currentAddress++, _axisSettings[Axis::Z][ValueType::MinValue]);
            EEPROM.update(currentAddress++, _axisSettings[Axis::Z][ValueType::MaxValue]);
            EEPROM.update(currentAddress++, _axisSettings[Axis::Z][ValueType::Center]);
            Serial.println(" Done");
            return true;
        }   
    }

    //Constructor with default settings to set if EEPROM values were not set
    JoyStick(int xPin, int yPin, int zPin, int xCenter, int yCenter, int zCenter, int xMinValue, int yMinValue, int zMinValue, int xMaxValue, int yMaxValue, int zMaxValue, int memoryStartAddress=-1) {
        setAxisValues(Axis::X, xPin, xCenter, xMinValue, xMaxValue);
        setAxisValues(Axis::Y, yPin, yCenter, yMinValue, yMaxValue);
        setAxisValues(Axis::Z, zPin, zCenter, zMinValue, zMaxValue);
        _memoryStartAddress = memoryStartAddress;
    }
};

#endif