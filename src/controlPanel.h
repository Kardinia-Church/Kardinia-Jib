/**
    Control Panel Handler
    Responsible for handling the user controls
**/

#ifndef CONTROLPANEL
#define CONTROLPANEL

#include <Arduino.h>
#include <EEPROMex.h>

#define TOTAL_ROWS 4
#define TOTAL_COLS 3
#define PINS_OFFSET 2

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
    struct Buttons { 
        int pins[TOTAL_ROWS][TOTAL_COLS];
        int buttonStates[TOTAL_ROWS][TOTAL_COLS]; 
    };
    private:
    int _memoryStartAddress;
    int _totalMemoryAllocation = CONTROLPANEL_MEM_ALLOC;
    int _potSettings[2][3] = {{0, 0, 0}, {0, 0, 0}};
    Buttons _buttons;

    //Calibrate a pot
    void calibratePot(Pot pot, LCD lcd, String currentPot = "") {
        int potValue = analogRead(_potSettings[pot][PotValues::Pin]);
        int minPotValue = 0;
        int maxPotValue = 1023;
        Serial.println("\n\nMove the pot to the center. Will start calibration in 5 seconds");
        lcd.showText(currentPot, "", " Move to centre");
        delay(5000);
        Serial.println(" Please set the POT to 0%");
        lcd.showText(currentPot, "", " Set to 0%");
        while(true) {
            if(analogRead(_potSettings[pot][PotValues::Pin]) >= potValue + 10 || analogRead(_potSettings[pot][PotValues::Pin]) <= potValue - 10) {
                delay(5000);
                Serial.println("  Getting the value");
                lcd.showText(currentPot, "", " Got it!");
                minPotValue = analogRead(_potSettings[pot][PotValues::Pin]);
                delay(1000);
                break;
            }
        }
        Serial.println(" Next set the POT to 100%");
        lcd.showText(currentPot, "", " Move to 100%");
        potValue = analogRead(_potSettings[pot][PotValues::Pin]);
        while(true) {
            if(analogRead(_potSettings[pot][PotValues::Pin]) >= potValue + 10 || analogRead(_potSettings[pot][PotValues::Pin]) <= potValue - 10) {
                delay(5000);
                Serial.println("  Getting the value");
                lcd.showText(currentPot, "", " Got it!");
                delay(1000);
                maxPotValue = analogRead(_potSettings[pot][PotValues::Pin]);
                break;
            }
        }

        _potSettings[pot][PotValues::Min] = minPotValue;
        _potSettings[pot][PotValues::Max] = maxPotValue;
        Serial.println("Setting pot values min=" + (String)minPotValue + ", max=" + (String)maxPotValue);
        lcd.showText(currentPot, "", " Complete");
        setSettingsToMemory();
    }

    public:
        //Constructor this assumes that the buttons are in a grid with the buttons going down each row before col
        ControlPanel(int startPin, int leftPotPin, int rightPotPin, int memoryStartAddress) { //probs no work
            _memoryStartAddress = memoryStartAddress;

            //Set the pins
            int currentPin = startPin;
            for(int i = TOTAL_ROWS - 1; i >= 0 ; i--) {
                for(int j = 0; j < TOTAL_COLS; j++) {
                    pinMode(currentPin, INPUT_PULLUP);
                    _buttons.pins[i][j] = currentPin;
                    _buttons.buttonStates[i][j] = false;
                    currentPin += PINS_OFFSET;
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
        Buttons isButtonsPressed() {
            for(int i = 0; i < TOTAL_ROWS; i++) {
                for(int j = 0; j < TOTAL_COLS; j++) {
                    int val = 0;
                    for(int k = 0; k < 20; k++) {
                        val += digitalRead(_buttons.pins[i][j]);
                    }
                    _buttons.buttonStates[i][j] = val / 20;
                }
            }

            return _buttons;



            // int buttonsDown[2][TOTAL_ROWS * TOTAL_COLS];
            // int totalButtonsDown = 0;
            // for(int i = 0; i < TOTAL_ROWS; i++) {
            //     for(int j = 0; j < TOTAL_COLS; j++) {
            //         if(digitalRead(_pins[i][j]) == 0) {
            //             if(_times[i][j] == 0) {_times[i][j] = millis() + 100;}
            //             else if(_times[i][j] < millis()) {
            //                 //This button has been pressed
            //                 _times[i][j] = 0;
            //                 buttonsDown[1][totalButtonsDown] = i * j;
            //                 totalButtonsDown++;
            //             }
            //         }
            //         else {_times[i][j] = 0;}
            //     }
            // }
            // buttonsDown[0][0] = totalButtonsDown;
        }

        //Get the percentage of a pot
        float getPotPercentage(Pot pot){
            return ((float)analogRead(_potSettings[pot][PotValues::Pin]) / (float)(_potSettings[pot][PotValues::Max] - _potSettings[pot][PotValues::Min])) * 100.0;
        }

        //Set the settings to memory
        boolean setSettingsToMemory() {
            if(_memoryStartAddress == -1 || _memoryStartAddress + _totalMemoryAllocation > END_OF_MEMORY) {
                Serial.println("Could not set control panel memory cause the start address was not set or is outside of useable memory");
                return false;
            }
            else {
                Serial.print("Set control panel to memory..");

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
                Serial.println("Could not read control panel memory cause the start address was not set or is outside of useable memory");
                return false;
            }
            else {
                //If all the values are set to 255 then set the defaults
                int amountOfUnsetData = 0;
                for(int i = _memoryStartAddress; i < _memoryStartAddress + _totalMemoryAllocation; i++) {if(EEPROM.read(i) == 255){amountOfUnsetData++;}}
                if(amountOfUnsetData == _totalMemoryAllocation) {
                    //Data is not set default them
                    Serial.println("Control panel settings are not set. Setting defaults");
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
        void calibrate(LCD leftLCD, LCD rightLCD) {
            rightLCD.showText("Calibration", "", "Follow left screen");
            Serial.println(" Calibration started for control panel");
            Serial.println(" \nThe left pot will be calibrated first");
            leftLCD.showText("Get Ready!", "", "Left Pot");
            delay(5000);
            calibratePot(Pot::Left, leftLCD, "Left Pot");
            delay(5000);
            Serial.println(" \nNext the right pot will be calibrated");
            leftLCD.showText("Get Ready!", "", "Right Pot");
            delay(5000);
            calibratePot(Pot::Right, leftLCD, "Right Pot");
            delay(5000);
            rightLCD.showText("Calibration", "", "Pots Complete");
            leftLCD.showText("Calibration", "", "Pots Complete");
            Serial.println(" Completed");
        }

};

#endif
