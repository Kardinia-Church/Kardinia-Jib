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


#include "lcd.h"

//LCD Settings
LCD leftLCD(0x3C);
LCD rightLCD(0x3D);

#include "joystick.h"
#include "controlPanel.h"
#include "head.h"
// #include "lancHandler.cpp"
#include "networkHandler.cpp"
#include "LibLanc/LibLanc.h"

//JoyStick Settings
JoyStick rightJoyStick(A5, A6, A7, true, true, false, RIGHTJOY_MEM_ADDR);

//Control panel settings
ControlPanel controlPanel(22, A0, A1, CONTROLPANEL_MEM_ADDR);

//Head Settings
//(AccelStepper stepper, int limitPin, boolean invert, int maxSpeed, int defaultAcceleration, long maxPosition, int invertLimit=0) {
Stepper xStepper(AccelStepper(AccelStepper::DRIVER, 3, 2), 46, 0, 1000, 50, 26000, 1);
Stepper yStepper(AccelStepper(AccelStepper::DRIVER, 5, 6), 48, 1, 2000, 50, 6500, 1);
Head head(xStepper, yStepper);

//Lanc settings
Lanc lanc(8, 9);

//Network settings
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 0, 0, 8);
IPAddress gateway(10, 0, 0, 138);
IPAddress subnet(255, 255, 255, 0);
NetworkHandler networkHandler(0, 3204, 3042, "tricaster", mac, ip, gateway, subnet);

#endif
