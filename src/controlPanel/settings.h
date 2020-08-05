/**
    Settings File
    Responsible for defining the settings of the system
**/

#ifndef SETTINGS_FILE
#define SETTINGS_FILE

#define DEBUG_LED 13

//Memory address allocations
#define MEMORY_LEAD_0 0x59
#define MEMORY_LEAD_1 0x45
#define MEMORY_LEAD_2 0x45
#define MEMORY_LEAD_3 0x54
#define MEMORY_END_0 0x48
#define MEMORY_END_1 0x49

#define JOY_MEM_ALLOC 18
#define RIGHTJOY_MEM_ADDR 4

#define CONTROLPANEL_MEM_ALLOC 8
#define CONTROLPANEL_MEM_ADDR RIGHTJOY_MEM_ADDR + JOY_MEM_ALLOC

// #define STEPPER_MEM_ALLOC 8
// #define HEAD_MEM_ALLOC STEPPER_MEM_ALLOC*2
// #define HEAD_MEM_ADDR CONTROLPANEL_MEM_ADDR + CONTROLPANEL_MEM_ALLOC
#define END_OF_MEMORY CONTROLPANEL_MEM_ADDR + CONTROLPANEL_MEM_ALLOC

#define INCOMING_PORT 23489
#define OUTGOING_PORT 21342
#define PACKET_SIZE 64

#include "lcd.h"

//LCD Settings
LCD leftLCD(0x3C);
LCD rightLCD(0x3D);

#include "joystick.h"
#include "controlPanel.h"
#include "head.h"
#include "lancHandler.cpp"
#include <UIPEthernet.h>


//JoyStick Settings
JoyStick rightJoyStick(A5, A6, A7, true, true, false, RIGHTJOY_MEM_ADDR);

//Control panel settings
ControlPanel controlPanel(22, A0, A1, CONTROLPANEL_MEM_ADDR);

//Head Settings
//(AccelStepper stepper, int limitPin, boolean invert, int maxSpeed, int defaultAcceleration, long maxPosition, int invertLimit=0) {
// Stepper xStepper(AccelStepper(AccelStepper::DRIVER, 3, 2), 50, 0, 1000, 50, 26000, 1);
// Stepper yStepper(AccelStepper(AccelStepper::DRIVER, 5, 4), 52, 1, 2000, 50, 6500, 1);

Stepper xStepper(AccelStepper(AccelStepper::DRIVER, 3, 4), 2, 0, 1000, 50, 26000, 1);
Stepper yStepper(AccelStepper(AccelStepper::DRIVER, 5, 6), 2, 1, 2000, 50, 6500, 1);
Head head(xStepper, yStepper);

// #include "../SerialCommunication.h"
// SerialCommunication serialCommunication(&Serial1);



byte mac[] = {0x1C, 0x39, 0x47, 0x10, 0x0A, 0x8F};
EthernetUDP udp;
String udpPassword = "tricaster";
bool foundServer = false;
IPAddress serverIP(192, 168, 0, 5);

#define STATIC_IP
#ifdef STATIC_IP
    IPAddress ip(10, 0, 0, 5);
    IPAddress gateway(10, 0, 0, 138);
    IPAddress subnet(255, 255, 255, 0);
#endif

LancController lancController(3, 4);


enum NetworkCommandType {
    Lanc,
    Head,
    Status
};

enum LancCommand {
    ZoomIn,
    ZoomOut,
    Rec,
    Focus,
    WhiteBalance,
    Iris,
    Shutter
};

enum LancZoomValue {
    Speed1,
    Speed2,
    Speed3,
    Speed4,
    Speed5,
    Speed6,
    Speed7,
    Speed8
};

enum LancRecValue {
    Start,
    Stop
};

enum LancFocusValue {
    Toggle,
    Far,
    Near
};

enum LancWhiteBalanceValue {
    Process,
    Reset
};

enum LancIrisValue {
    Auto,
    Open,
    Close
};

enum LancShutterValue {
    Next
};

enum StatusValue {
    Startup,
    Ready,
    NetworkDisconnected,
    NetworkConnected
};

#endif
