/*
Kardinia Jib Lanc Handler By Kardinia Church 2020
"LANC" is a registered trademark of SONY.
CANON calls their LANC compatible port "REMOTE".

lancHandler.cpp: Provides control of the LANC protocol
*/

#include <Arduino.h>

#define REPEAT 3
#define BIT_DELAY 104
#define TIMEOUT 10

class LancController {
    private:
    int _outputPin;
    int _inputPin;

    //Write a byte
    void sendByte(int byte) {
        delayMicroseconds(BIT_DELAY);
        for(int j = 0; j < 8; j++) {
            digitalWrite(_outputPin, bitRead(byte, j));
            delayMicroseconds(BIT_DELAY - 8);
        }
    }

    //Attempts to send a raw command to the camera
    //Returns true if successful, false is not successful
    bool sendLancCommand(int subCmd, int cmd) {
        //Repeat sending the packet a few times to ensure the camera got it
        for(int i = 0; i < REPEAT; i++) {
            //Wait for sync
            unsigned long timeout = millis() + TIMEOUT;
            while(pulseIn(_inputPin, 1) < 5000) {
                //Wait for the camera's sync pulse if it's not found return false
                if(timeout < millis()){return false;}
            }

            //First write the sub command
            sendByte(subCmd);
            digitalWrite(_outputPin, LOW);
            delayMicroseconds(10);

            //Wait for line to low to send next part
            timeout = millis() + TIMEOUT;
            while(digitalRead(_inputPin)) {
                //Wait for the camera's sync pulse if it's not found return false
                if(timeout < millis()){return false;}
            }

            //Next write the command
            sendByte(cmd);
            digitalWrite(_outputPin, LOW);
        }

        return true;
    };
    public:
    enum CameraCommand {
        //Sub command 0x18
        startStop,
        photoCapture,
        rec,
        recPause,
        powerOff,
        sleep,

        //Sub command 0x28
        zoomTele01,
        zoomTele02,
        zoomTele03,
        zoomTele04,
        zoomTele05,
        zoomTele06,
        zoomTele07,
        zoomTele08,
        zoomWide01,
        zoomWide02,
        zoomWide03,
        zoomWide04,
        zoomWide05,
        zoomWide06,
        zoomWide07,
        zoomWide08,
        recStart,
        recStop,
        autoFocusToggle,
        focusFar,
        focusNear,
        whiteBalanceToggle,
        exposureAutoManualToggle,
        irisClose,
        irisOpen,
        shutter,
        whiteBalanceReset,
        irisAuto

    };
    LancController(int inputPin, int outputPin) {
        _inputPin = inputPin;
        _outputPin = outputPin;
        pinMode(_outputPin, OUTPUT);
        pinMode(_inputPin, INPUT);
    }

    //Send out a raw command
    //Returns true if successful, false is failed (camera did not respond)
    bool sendRaw(int subCmd, int cmd) {
        return sendLancCommand(subCmd, cmd);
    }

    //Sends out a specific command
    //Returns true if successful, false is failed (camera did not respond)
    bool sendCommand(CameraCommand command) {
        int subCmd = 0;
        int cmd = 0;

        Serial.print(" ID=" + command);

        switch(command) {
            //Sub command 0x18
            case CameraCommand::startStop: {subCmd = 0x18; cmd = 0x33; break;}
            case CameraCommand::photoCapture: {subCmd = 0x18; cmd = 0x39; break;}
            case CameraCommand::rec: {subCmd = 0x18; cmd = 0x3A; break;}
            case CameraCommand::recPause: {subCmd = 0x18; cmd = 0x3C; break;}
            case CameraCommand::powerOff: {subCmd = 0x18; cmd = 0x5E; break;}
            case CameraCommand::sleep: {subCmd = 0x18; cmd = 0x6C; break;}

            //Sub command 0x28
            case CameraCommand::zoomTele01: {subCmd = 0x28; cmd = 0x00; break;}
            case CameraCommand::zoomTele02: {subCmd = 0x28; cmd = 0x02; break;}
            case CameraCommand::zoomTele03: {subCmd = 0x28; cmd = 0x04; break;}
            case CameraCommand::zoomTele04: {subCmd = 0x28; cmd = 0x06; break;}
            case CameraCommand::zoomTele05: {subCmd = 0x28; cmd = 0x08; break;}
            case CameraCommand::zoomTele06: {subCmd = 0x28; cmd = 0x0A; break;}
            case CameraCommand::zoomTele07: {subCmd = 0x28; cmd = 0x0C; break;}
            case CameraCommand::zoomTele08: {subCmd = 0x28; cmd = 0x0E; break;}
            case CameraCommand::zoomWide01: {subCmd = 0x28; cmd = 0x10; break;}
            case CameraCommand::zoomWide02: {subCmd = 0x28; cmd = 0x12; break;}
            case CameraCommand::zoomWide03: {subCmd = 0x28; cmd = 0x14; break;}
            case CameraCommand::zoomWide04: {subCmd = 0x28; cmd = 0x16; break;}
            case CameraCommand::zoomWide05: {subCmd = 0x28; cmd = 0x18; break;}
            case CameraCommand::zoomWide06: {subCmd = 0x28; cmd = 0x1A; break;}
            case CameraCommand::zoomWide07: {subCmd = 0x28; cmd = 0x1C; break;}
            case CameraCommand::zoomWide08: {subCmd = 0x28; cmd = 0x1E; break;}
            case CameraCommand::recStart: {subCmd = 0x28; cmd = 0x27; break;}
            case CameraCommand::recStop: {subCmd = 0x28; cmd = 0x29; break;}
            case CameraCommand::autoFocusToggle: {subCmd = 0x28; cmd = 0x41; break;}
            case CameraCommand::focusFar: {subCmd = 0x28; cmd = 0x45; break;}
            case CameraCommand::focusNear: {subCmd = 0x28; cmd = 0x47; break;}
            case CameraCommand::whiteBalanceToggle: {subCmd = 0x28; cmd = 0x49; break;}
            case CameraCommand::exposureAutoManualToggle: {subCmd = 0x28; cmd = 0x53; break;}
            case CameraCommand::irisClose: {subCmd = 0x28; cmd = 0x54; break;} //i think
            case CameraCommand::irisOpen: {subCmd = 0x28; cmd = 0x55; break;}
            case CameraCommand::shutter: {subCmd = 0x28; cmd = 0x61; break;}
            case CameraCommand::whiteBalanceReset: {subCmd = 0x28; cmd = 0x77; break;}
            case CameraCommand::irisAuto: {subCmd = 0x28; cmd = 0xAF; break;}
        }

        return sendLancCommand(subCmd, cmd);
    };
};