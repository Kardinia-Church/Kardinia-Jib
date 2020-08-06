/*
Kardinia Jib By Kardinia Church 2020
"LANC" is a registered trademark of SONY.
CANON calls their LANC compatible port "REMOTE".

lancHandler.cpp: Provides control of the LANC protocol
*/

#include <Arduino.h>
#include "LibLanc/LibLanc.h"

#define REPEAT 3
#define BIT_DELAY 104
#define TIMEOUT 10

class LancController {
    private:
    Lanc *lanc;
    public:
    LancController(int inputPin, int outputPin) {
        lanc = new Lanc(inputPin, outputPin);
    }

    //Process the loop
    void process() {lanc->loop();}

    //Perform a zoom
    void performZoom(int stepSize) {
        lanc->Zoom(stepSize);
    }


    // //Send out a raw command
    // //Returns true if successful, false is failed (camera did not respond)
    // bool sendRaw(int subCmd, int cmd) {
    //     return sendLancCommand(subCmd, cmd);
    // }

//     //Sends out a specific command
//     //Returns true if successful, false is failed (camera did not respond)
//     bool sendCommand(CameraCommand command) {
//         int subCmd = 0;
//         int cmd = 0;
// ;
//         switch(command) {
//             //Sub command 0x18
//             case CameraCommand::startStop: {subCmd = 0x18; cmd = 0x33; break;}
//             case CameraCommand::photoCapture: {subCmd = 0x18; cmd = 0x39; break;}
//             case CameraCommand::rec: {subCmd = 0x18; cmd = 0x3A; break;}
//             case CameraCommand::recPause: {subCmd = 0x18; cmd = 0x3C; break;}
//             case CameraCommand::powerOff: {subCmd = 0x18; cmd = 0x5E; break;}
//             case CameraCommand::sleep: {subCmd = 0x18; cmd = 0x6C; break;}

//             //Sub command 0x28
//             case CameraCommand::zoomTele01: {subCmd = 0x28; cmd = 0x00; break;}
//             case CameraCommand::zoomTele02: {subCmd = 0x28; cmd = 0x02; break;}
//             case CameraCommand::zoomTele03: {subCmd = 0x28; cmd = 0x04; break;}
//             case CameraCommand::zoomTele04: {subCmd = 0x28; cmd = 0x06; break;}
//             case CameraCommand::zoomTele05: {subCmd = 0x28; cmd = 0x08; break;}
//             case CameraCommand::zoomTele06: {subCmd = 0x28; cmd = 0x0A; break;}
//             case CameraCommand::zoomTele07: {subCmd = 0x28; cmd = 0x0C; break;}
//             case CameraCommand::zoomTele08: {subCmd = 0x28; cmd = 0x0E; break;}
//             case CameraCommand::zoomWide01: {subCmd = 0x28; cmd = 0x10; break;}
//             case CameraCommand::zoomWide02: {subCmd = 0x28; cmd = 0x12; break;}
//             case CameraCommand::zoomWide03: {subCmd = 0x28; cmd = 0x14; break;}
//             case CameraCommand::zoomWide04: {subCmd = 0x28; cmd = 0x16; break;}
//             case CameraCommand::zoomWide05: {subCmd = 0x28; cmd = 0x18; break;}
//             case CameraCommand::zoomWide06: {subCmd = 0x28; cmd = 0x1A; break;}
//             case CameraCommand::zoomWide07: {subCmd = 0x28; cmd = 0x1C; break;}
//             case CameraCommand::zoomWide08: {subCmd = 0x28; cmd = 0x1E; break;}
//             case CameraCommand::recStart: {subCmd = 0x28; cmd = 0x27; break;}
//             case CameraCommand::recStop: {subCmd = 0x28; cmd = 0x29; break;}
//             case CameraCommand::autoFocusToggle: {subCmd = 0x28; cmd = 0x41; break;}
//             case CameraCommand::focusFar: {subCmd = 0x28; cmd = 0x45; break;}
//             case CameraCommand::focusNear: {subCmd = 0x28; cmd = 0x47; break;}
//             case CameraCommand::whiteBalanceToggle: {subCmd = 0x28; cmd = 0x49; break;}
//             case CameraCommand::exposureAutoManualToggle: {subCmd = 0x28; cmd = 0x53; break;}
//             case CameraCommand::irisClose: {subCmd = 0x28; cmd = 0x54; break;} //i think
//             case CameraCommand::irisOpen: {subCmd = 0x28; cmd = 0x55; break;}
//             case CameraCommand::shutter: {subCmd = 0x28; cmd = 0x61; break;}
//             case CameraCommand::whiteBalanceReset: {subCmd = 0x28; cmd = 0x77; break;}
//             case CameraCommand::irisAuto: {subCmd = 0x28; cmd = 0xAF; break;}
//         }

//         return sendLancCommand(subCmd, cmd);
//     };
};