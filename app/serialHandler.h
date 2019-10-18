/**
    Serial handler
    Responsible for handling commands sent via the serial port
**/

#ifndef SERIAL_FILE
#define SERIAL_FILE

#include <Arduino.h>

class SerialMenuHandler {
    private:
    String mainMenuActions[2] = {"Calibrate", "Home Axis"};

    public:
    void printMainMenu() {
        Serial.println("------   Main Menu   -------");
        for(int i = 0; i < 2; i++) {
            Serial.println(String(i) + " = " + mainMenuActions[i]);
        }
    }

    //Main listener for incoming commands
    void listen() {
        if (Serial.available() > 0) {
            Serial.println(Serial.read(), DEC);
        }
    }

    SerialMenuHandler(){}
};
#endif