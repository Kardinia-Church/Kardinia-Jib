/**
    Control Panel Handler
    Responsible for handling the user controls
**/

#ifndef CONTROLPANEL
#define CONTROLPANEL

#include <Arduino.h>
#include <EEPROMex.h>

#define TOTAL_ROWS 5
#define TOTAL_COLS 5

class ControlPanel {
    public:
    enum PotValues {
        Pin,
        Min,
        Max,
    };
    enum Pot {
        Left,
        Right
    };
    private:
    int _memoryStartAddress;
    int _totalMemoryAllocation = CONTROLPANEL_MEM_ALLOC;
    int _pins[TOTAL_ROWS][TOTAL_COLS];
    long _times[TOTAL_ROWS][TOTAL_COLS];
    int _potSettings[2][3] = {{0, 0, 0}, {0, 0, 0}};

    //Calibrate a pot
    void calibratePot(Pot pot) {
        int potValue = analogRead(_potSettings[pot][PotValues::Pin]);
        int minPotValue = 0;
        int maxPotValue = 1023;
        Serial.println("[CAL] Move the pot to the center. Will start calibration in 5 seconds");
        delay(5000);
        Serial.println("[CAL] Please set the POT to 0%");
        while(true) {
            if(analogRead(_potSettings[pot][PotValues::Pin]) >= potValue + 10 || analogRead(_potSettings[pot][PotValues::Pin]) <= potValue - 10) {
                Serial.println("[CAL] Getting the value");
                delay(5000);
                minPotValue = analogRead(_potSettings[pot][PotValues::Pin]);
                break;
            }
        }
        Serial.println("[CAL] Next set the POT to 100%");
        potValue = analogRead(_potSettings[pot][PotValues::Pin]);
        while(true) {
            if(analogRead(_potSettings[pot][PotValues::Pin]) >= potValue + 10 || analogRead(_potSettings[pot][PotValues::Pin]) <= potValue - 10) {
                Serial.println("[CAL] Getting the value");
                delay(5000);
                maxPotValue = analogRead(_potSettings[pot][PotValues::Pin]);
                break;
            }
        }

        _potSettings[pot][PotValues::Min] = minPotValue;
        _potSettings[pot][PotValues::Max] = maxPotValue;
        Serial.println("[CAL] Setting pot values min=" + (String)minPotValue + ", max=" + (String)maxPotValue);
        setSettingsToMemory();
    }

    public:
        //Constructor this assumes that the buttons are in a grid with the buttons going down each row before col
        ControlPanel(int startPin, int leftPotPin, int rightPotPin, int memoryStartAddress) { //probs no work
            _memoryStartAddress = memoryStartAddress;

            //Set the pins
            int currentPin = startPin;
            for(int i = 0; i < TOTAL_ROWS; i++) {
                for(int j = 0; j < TOTAL_COLS; j++) {
                    pinMode(currentPin, INPUT_PULLUP);
                    _pins[i][j] = ++currentPin;
                }
            }

            _potSettings[Pot::Left][PotValues::Pin] = leftPotPin;
            _potSettings[Pot::Right][PotValues::Pin] = rightPotPin;
            pinMode(_potSettings[Pot::Left][PotValues::Pin], INPUT);
            pinMode(_potSettings[Pot::Right][PotValues::Pin], INPUT);
        }

        //Constantly check if a button is pressed. This should be called every loop cycle. Will return an array of button id's that have been pressed 
        //[0][0] is the amount of buttons down
        //[1][x..amountOfButtonsDown] is the button id(s) that are down
        int *isButtonsPressed() {
            int buttonsDown[2][TOTAL_ROWS * TOTAL_COLS];
            int totalButtonsDown = 0;
            for(int i = 0; i < TOTAL_ROWS; i++) {
                for(int j = 0; j < TOTAL_COLS; j++) {
                    if(digitalRead(_pins[i][j]) == 0) {
                        if(_times[i][j] == 0) {_times[i][j] = millis() + 100;}
                        else if(_times[i][j] < millis()) {
                            //This button has been pressed
                            _times[i][j] = 0;
                            buttonsDown[1][totalButtonsDown] = i * j;
                            totalButtonsDown++;
                        }
                    }
                    else {_times[i][j] = 0;}
                }
            }
            buttonsDown[0][0] = totalButtonsDown;
        }

        //Get the percentage of a pot
        float getPotPercentage(Pot pot){
            return ((float)analogRead(_potSettings[pot][PotValues::Pin]) / (float)(_potSettings[pot][PotValues::Max] - _potSettings[pot][PotValues::Min])) * 100.0;
        }

        //Set the settings to memory
        boolean setSettingsToMemory() {
            if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > END_OF_MEMORY) {
                Serial.println("[ERROR] Could not set control panel memory cause the start address was not set or is outside of useable memory");
                return false;
            }
            else {
                Serial.print("[INFO] Set control panel to memory..");

                Serial.print("LeftPot@" + (String)_memoryStartAddress + " , ");
                EEPROM.writeInt(_memoryStartAddress, _potSettings[Pot::Left][PotValues::Min]);
                EEPROM.writeInt(_memoryStartAddress + 2, _potSettings[Pot::Left][PotValues::Max]);

                Serial.print("RightPot@" + (String)(_memoryStartAddress + 4) + " , ");
                EEPROM.writeInt(_memoryStartAddress + 4, _potSettings[Pot::Right][PotValues::Min]);
                EEPROM.writeInt(_memoryStartAddress + 6, _potSettings[Pot::Right][PotValues::Max]);
                Serial.println(" Done");
                return true;
            } 
        }

        //Read the settings from memory
        boolean readSettingsFromMemory() {
            if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > END_OF_MEMORY) {
                Serial.println("[ERROR] Could not read control panel memory cause the start address was not set or is outside of useable memory");
                return false;
            }
            else {
                //If all the values are set to 255 then set the defaults
                int amountOfUnsetData = 0;
                for(int i = _memoryStartAddress; i < _memoryStartAddress + _totalMemoryAllocation; i++) {if(EEPROM.read(i) == 255){amountOfUnsetData++;}}
                if(amountOfUnsetData == _totalMemoryAllocation) {
                    //Data is not set default them
                    Serial.println("[WARN] Control panel settings are not set. Setting defaults");
                    setSettingsToMemory();
                    return true;
                }
                else {
                    _potSettings[Pot::Left][PotValues::Min] = EEPROM.readInt(_memoryStartAddress);
                    _potSettings[Pot::Left][PotValues::Max] = EEPROM.readInt(_memoryStartAddress + 2);

                    _potSettings[Pot::Right][PotValues::Min] = EEPROM.readInt(_memoryStartAddress + 4);
                    _potSettings[Pot::Right][PotValues::Max] = EEPROM.readInt(_memoryStartAddress + 6);
                    return true;
                }
            }
        }

        //Check that the settings are valid
        boolean checkSettings() {
            if(_potSettings[Pot::Left][PotValues::Min] == 0 && _potSettings[Pot::Left][PotValues::Max] == 0){return false;}
            if(_potSettings[Pot::Right][PotValues::Min] == 0 && _potSettings[Pot::Right][PotValues::Max] == 0){return false;}
            return true;
        }

        //Print out the set settings
        void printSettings() {
            Serial.print("LeftPot[Min, Max]=" + (String)_potSettings[Pot::Left][PotValues::Min] + "," + (String)_potSettings[Pot::Left][PotValues::Max] + "; ");
            Serial.print("RightPot[Min, Max,]=" + (String)_potSettings[Pot::Right][PotValues::Min] + "," + (String)_potSettings[Pot::Right][PotValues::Max] + "; ");
            Serial.println("");
        }

        //Calibrate
        void calibrate() {
        rightLCD.showError("Connect PC", "Please connect", "serial to calibrate");
        while(Serial.available() == 1){
            digitalWrite(DEBUG_LED, 1);
            delay(100);
            digitalWrite(DEBUG_LED, 0);
            delay(100);
        }

        rightLCD.showError("Calibrating", "Follow messages on", "serial to calibrate");

            Serial.println("[CAL] Calibration started for control panel");
            Serial.println("[CAL] The left pot will be calibrated first");
            calibratePot(Pot::Left);
            Serial.println("[CAL] Next the right pot will be calibrated");
            calibratePot(Pot::Right);
            Serial.println("[CAL] Completed");
        }

};

#endif
