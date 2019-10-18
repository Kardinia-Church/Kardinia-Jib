/**
    Main Joystick Handler
    Responsible for handling the joystick

    There is something hardware wise wrong with the z axis
**/

#ifndef JOYSTICK_HANDLER
#define JOYSTICK_HANDLER

#include <Arduino.h>
#include <EEPROMex.h>
#include "settings.h"

class JoyStick {
    public:
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
    private:
    int _axisSettings[3][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
    int _memoryStartAddress = 1;
    int _totalMemoryAllocation = JOY_MEM_ALLOC;
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

    //Get the center value of a axis. Returns the center value
    int calibateAxisCenter(Axis axis) {
        int centerValue = 0;
        for(int i = 0; i < 10; i++) {
            centerValue += analogRead(_axisSettings[axis][ValueType::Pin]);
            delay(100);
        }
        return centerValue / 10;
    }

    //Get the min max values of a axis. Returns array [min, max]
    void calibateAxisMinMax(Axis axis, int &min, int &max) {
        min = 10000;
        max = 0;
        long timeout = millis() + 5000;
        while(true) {
            if(millis() > timeout){break;}
            int read = analogRead(_axisSettings[axis][ValueType::Pin]);
            if(read < min) {min = read;}
            if(read > max) {max = read;}
        }
    }

    //Calibrate the joystick
    void calibrate() {
        while(Serial.available() == 1){
            digitalWrite(DEBUG_LED, 1);
            delay(100);
            digitalWrite(DEBUG_LED, 0);
            delay(100);
        }

        int values[3][3];
        Serial.println("[CAL] Calibration started for joystick.");

        Serial.println("[CAL] We will calibrate the axis in the following order: X=0, Y=1, Z=2");
        for(int i = Axis::X; i <= Axis::Z; i++) {
            Serial.println("[CAL] Calibration start for axis: " + (String)i);
            //Center
            Serial.println("[CAL] Please let the axis sit at its center point. Will start in 5 seconds");
            delay(5000);
            Serial.print("[CAL] Begin calibation of center point..");
            values[i][0] = calibateAxisCenter((Axis)i);
            Serial.println(" Done");
            //Min max
            Serial.println("[CAL] Please move the axis from its min point to its max point. Will start in 5 seconds");
            delay(5000);
            Serial.println("[CAL] Begin calibation of min max..");
            int min = 0;
            int max = 0;
            calibateAxisMinMax((Axis)i, min, max);
            values[i][1] = min;
            values[i][2] = max;
            Serial.println("[CAL] Completed calibration of axis " + (String)i);
        }

        Serial.println("[CAL] Calibration complete. Values read:");
        Serial.println("[CAL] X: Center=" + (String)values[Axis::X][0] + "Min=" + (String)values[Axis::X][1] + "Max=" + (String)values[Axis::X][2]);
        Serial.println("[CAL] Y: Center=" + (String)values[Axis::Y][0] + "Min=" + (String)values[Axis::Y][1] + "Max=" + (String)values[Axis::Y][2]);
        Serial.println("[CAL] Z: Center=" + (String)values[Axis::Z][0] + "Min=" + (String)values[Axis::Z][1] + "Max=" + (String)values[Axis::Z][2]);
        Serial.println("[CAL] Setting calibation data to memory");
        _axisSettings[Axis::X][ValueType::Center] = values[Axis::X][0];
        _axisSettings[Axis::X][ValueType::MinValue] = values[Axis::X][1];
        _axisSettings[Axis::X][ValueType::MaxValue] = values[Axis::X][2];
        _axisSettings[Axis::Y][ValueType::Center] = values[Axis::Y][0];
        _axisSettings[Axis::Y][ValueType::MinValue] = values[Axis::Y][1];
        _axisSettings[Axis::Y][ValueType::MaxValue] = values[Axis::Y][2];
        _axisSettings[Axis::Z][ValueType::Center] = values[Axis::Z][0];
        _axisSettings[Axis::Z][ValueType::MinValue] = values[Axis::Z][1];
        _axisSettings[Axis::Z][ValueType::MaxValue] = values[Axis::Z][2];
        setSettingsToMemory();
        Serial.println("[CAL] - Calibration complete!");
    }

    //Read the settings from memory XYZ[min, max, center]
    boolean readSettingsFromMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > END_OF_MEMORY) {
            Serial.println("[ERROR] Could not read joystick memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            //If all the values are set to 255 then set the defaults
            int amountOfUnsetData = 0;
            for(int i = _memoryStartAddress; i < _memoryStartAddress + _totalMemoryAllocation; i++) {if(EEPROM.read(i) == 255){amountOfUnsetData++;}}
            if(amountOfUnsetData == _totalMemoryAllocation) {
                //Data is not set default them
                Serial.println("[WARN] Joystick settings are not set. Setting defaults");
                setSettingsToMemory();
                return true;
            }
            else {
                _axisSettings[Axis::X][ValueType::MinValue] = EEPROM.readInt(_memoryStartAddress);
                _axisSettings[Axis::X][ValueType::MaxValue] = EEPROM.readInt(_memoryStartAddress + 2);
                _axisSettings[Axis::X][ValueType::Center] = EEPROM.readInt(_memoryStartAddress + 4);

                _axisSettings[Axis::Y][ValueType::MinValue] = EEPROM.readInt(_memoryStartAddress + 6);
                _axisSettings[Axis::Y][ValueType::MaxValue] = EEPROM.readInt(_memoryStartAddress + 8);
                _axisSettings[Axis::Y][ValueType::Center] = EEPROM.readInt(_memoryStartAddress + 10);

                _axisSettings[Axis::Z][ValueType::MinValue] = EEPROM.readInt(_memoryStartAddress + 12);
                _axisSettings[Axis::Z][ValueType::MaxValue] = EEPROM.readInt(_memoryStartAddress + 14);
                _axisSettings[Axis::Z][ValueType::Center] = EEPROM.readInt(_memoryStartAddress + 16);
                return true;
            }
        }
    }

    //Set the settings from memory
    boolean setSettingsToMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > END_OF_MEMORY) {
            Serial.println("[ERROR] Could not set joystick memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            Serial.print("[INFO] Set joystick to memory..");

            Serial.print("X@" + (String)_memoryStartAddress + " , ");
            EEPROM.writeInt(_memoryStartAddress, _axisSettings[Axis::X][ValueType::MinValue]);
            EEPROM.writeInt(_memoryStartAddress + 2, _axisSettings[Axis::X][ValueType::MaxValue]);
            EEPROM.writeInt(_memoryStartAddress + 4, _axisSettings[Axis::X][ValueType::Center]);

            Serial.print("Y@" + (String)(_memoryStartAddress + 6) + " , ");
            EEPROM.writeInt(_memoryStartAddress + 6, _axisSettings[Axis::Y][ValueType::MinValue]);
            EEPROM.writeInt(_memoryStartAddress + 8, _axisSettings[Axis::Y][ValueType::MaxValue]);
            EEPROM.writeInt(_memoryStartAddress + 10, _axisSettings[Axis::Y][ValueType::Center]);

            Serial.print("Z@" + (String)(_memoryStartAddress + 12));
            EEPROM.writeInt(_memoryStartAddress + 12, _axisSettings[Axis::Z][ValueType::MinValue]);
            EEPROM.writeInt(_memoryStartAddress + 14, _axisSettings[Axis::Z][ValueType::MaxValue]);
            EEPROM.writeInt(_memoryStartAddress + 16, _axisSettings[Axis::Z][ValueType::Center]);
            Serial.println(" Done");
            return true;
        }  
    }

    //Check that the settings are valid
    boolean checkSettings() {
        if(_axisSettings[Axis::X][ValueType::MinValue] == 0 && _axisSettings[Axis::X][ValueType::Center] == 0 && _axisSettings[Axis::X][ValueType::MaxValue] == 0){return false;}
        if(_axisSettings[Axis::Y][ValueType::MinValue] == 0 && _axisSettings[Axis::Y][ValueType::Center] == 0 && _axisSettings[Axis::Y][ValueType::MaxValue] == 0){return false;}
        if(_axisSettings[Axis::Z][ValueType::MinValue] == 0 && _axisSettings[Axis::Z][ValueType::Center] == 0 && _axisSettings[Axis::Z][ValueType::MaxValue] == 0){return false;}
        return true;
    }

    //Print out the set settings
    void printSettings() {
        Serial.print("X[Min, Max, Center]=" + (String)_axisSettings[Axis::X][ValueType::MinValue] + "," + (String)_axisSettings[Axis::X][ValueType::MaxValue] + "," + (String)_axisSettings[Axis::X][ValueType::Center] + "; ");
        Serial.print("Y[Min, Max, Center]=" + (String)_axisSettings[Axis::Y][ValueType::MinValue] + "," + (String)_axisSettings[Axis::Y][ValueType::MaxValue] + "," + (String)_axisSettings[Axis::Y][ValueType::Center] + "; ");
        Serial.print("Z[Min, Max, Center]=" + (String)_axisSettings[Axis::Z][ValueType::MinValue] + "," + (String)_axisSettings[Axis::Z][ValueType::MaxValue] + "," + (String)_axisSettings[Axis::Z][ValueType::Center] + "; ");
        Serial.println("");
    }

    //Constructor with default settings to set if EEPROM values were not set
    JoyStick(int xPin, int yPin, int zPin, int memoryStartAddress=-1) {
        _axisSettings[Axis::X][ValueType::Pin] = xPin;
        _axisSettings[Axis::Y][ValueType::Pin] = yPin;
        _axisSettings[Axis::Z][ValueType::Pin] = zPin;
        pinMode(xPin, INPUT);
        pinMode(yPin, INPUT);
        pinMode(zPin, INPUT);
        _memoryStartAddress = memoryStartAddress;
    }
};

#endif