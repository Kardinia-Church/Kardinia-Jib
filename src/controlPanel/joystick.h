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
        MaxValue,
        Invert
    };
    enum Axis {
        X,
        Y,
        Z
    };
    private:
    int _axisSettings[3][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};
    int _memoryStartAddress = 1;
    int _totalMemoryAllocation = JOY_MEM_ALLOC;
    const int _deadzone = 20;

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

        if(_axisSettings[axis][ValueType::Invert] == 1) {
          return -val;
        }
        else {
          return val;
        }
    }

    //Get the percentage of a axis
    float getPercentage(Axis axis) {
        float divider = _axisSettings[axis][ValueType::MaxValue] - _axisSettings[axis][ValueType::MinValue];
        float val = ((float)getValue(axis)/divider) * 100.0;
        if(val < -100.0){val = -100.0;}
        if(val > 100.0) {val = 100.0;}
        return val * 2;
    }

    //Output the joystick values to console
    void joystickDebug() {
        int value = 0;
        float percentage = 0.0;
        Serial.print("Joystick Raw XYZ:");
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
    void calibrate(LCD leftLCD, LCD rightLCD) {
        leftLCD.showText("Joystick", "", "Get ready");
        rightLCD.showText("Calibration", "Follow left screen");
        delay(5000);

        int values[3][3];
        Serial.println("Calibration started for joystick.");

        Serial.println("We will calibrate the axis in the following order: X=0, Y=1, Z=2");
        leftLCD.showText("Get Ready!", "", "Order is X, Y, Z");
        delay(5000);
        for(int i = Axis::X; i <= Axis::Z; i++) {
            String axis = "";
            switch(i) {
                case Axis::X: {axis = "X"; break;}
                case Axis::Y: {axis = "Y"; break;}
                case Axis::Z: {axis = "Z"; break;}
            }
            Serial.println("Calibration start for axis: " + axis);

            //Center
            Serial.println("Please let the axis sit at its center point. Will start in 5 seconds");
            leftLCD.showText(axis + " Centre", "", "Let " + axis + " sit");
            delay(5000);
            Serial.print("Begin calibation of center point..");
            values[i][0] = calibateAxisCenter((Axis)i);
            Serial.println(" Done");
            leftLCD.showText(axis + " Centre", "", "Got it!");
            delay(1000);

            //Min max
            Serial.println("Please move the axis from its min point to its max point. Will start in 5 seconds");
            leftLCD.showText(axis + " Min-Max", "", "Move to min-max");
            delay(5000);
            Serial.println("Begin calibation of min max..");
            int min = 0;
            int max = 0;
            calibateAxisMinMax((Axis)i, min, max);
            values[i][1] = min;
            values[i][2] = max;
            Serial.println("Completed calibration of axis " + (String)i);
            leftLCD.showText(axis + " Min-Max", "", "Got it!");
            delay(1000);
            leftLCD.showText(axis + " Complete", "", "");
            delay(5000);
        }

        Serial.println("Calibration complete. Values read:");
        Serial.println("X: Center=" + (String)values[Axis::X][0] + "Min=" + (String)values[Axis::X][1] + "Max=" + (String)values[Axis::X][2]);
        Serial.println("Y: Center=" + (String)values[Axis::Y][0] + "Min=" + (String)values[Axis::Y][1] + "Max=" + (String)values[Axis::Y][2]);
        Serial.println("Z: Center=" + (String)values[Axis::Z][0] + "Min=" + (String)values[Axis::Z][1] + "Max=" + (String)values[Axis::Z][2]);
        Serial.println("Setting calibation data to memory");
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

        rightLCD.showText("Calibration", "", "Joystick Complete");
        leftLCD.showText("Calibration", "", "Joystick Complete");
        Serial.println("Calibration complete!");
        delay(5000);
    }

    //Read the settings from memory XYZ[min, max, center]
    boolean readSettingsFromMemory() {
        if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > END_OF_MEMORY) {
            Serial.println("Could not read joystick memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            //If all the values are set to 255 then set the defaults
            int amountOfUnsetData = 0;
            for(int i = _memoryStartAddress; i < _memoryStartAddress + _totalMemoryAllocation; i++) {if(EEPROM.read(i) == 255){amountOfUnsetData++;}}
            if(amountOfUnsetData == _totalMemoryAllocation) {
                //Data is not set default them
                Serial.println("Joystick settings are not set. Setting defaults");
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
            Serial.println("Could not set joystick memory cause the start address was not set or is outside of useable memory");
            return false;
        }
        else {
            Serial.print("Set joystick to memory..");

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

    //If one of a axis is not at 0 returns true
    boolean isActive() {
        if(getPercentage(Axis::X) != 0.0){return true;}
        if(getPercentage(Axis::Y) != 0.0){return true;}
        if(getPercentage(Axis::Z) != 0.0){return true;}
        return false;
    }

    //Constructor with default settings to set if EEPROM values were not set
    JoyStick(int xPin, int yPin, int zPin, bool invertX=false, bool invertY=false, bool invertZ=false, int memoryStartAddress=-1) {
        _axisSettings[Axis::X][ValueType::Pin] = xPin;
        _axisSettings[Axis::Y][ValueType::Pin] = yPin;
        _axisSettings[Axis::Z][ValueType::Pin] = zPin;
        _axisSettings[Axis::X][ValueType::Invert] = invertX;
        _axisSettings[Axis::Y][ValueType::Invert] = invertY;
        _axisSettings[Axis::Z][ValueType::Invert] = invertZ;
        pinMode(xPin, INPUT);
        pinMode(yPin, INPUT);
        pinMode(zPin, INPUT);
        _memoryStartAddress = memoryStartAddress;
    }
};

#endif
