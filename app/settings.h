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

#define STEPPER_MEM_ALLOC 8
#define HEAD_MEM_ALLOC STEPPER_MEM_ALLOC*2
#define HEAD_MEM_ADDR CONTROLPANEL_MEM_ADDR + CONTROLPANEL_MEM_ALLOC
#define END_OF_MEMORY HEAD_MEM_ADDR + HEAD_MEM_ALLOC

#include "lcd.h"

//LCD Settings
LCD leftLCD(0x3C);
LCD rightLCD(0x3D);

#include "joystick.h"
#include "head.h"
#include "controlPanel.h"

//JoyStick Settings
JoyStick rightJoyStick(A5, A6, A7, RIGHTJOY_MEM_ADDR);

//Control panel settings
ControlPanel controlPanel(22, A0, A1, CONTROLPANEL_MEM_ADDR);

//Head Settings
//Stepper(AccelStepper stepper(int stepPin, int directionPin), int limitPin, boolean invert, int maxSpeed, int defaultAcceleration, int invertLimit=0) {
Stepper xStepper(AccelStepper(AccelStepper::DRIVER, 3, 2), 50, 0, 2000, 3000, 1);
Stepper yStepper(AccelStepper(AccelStepper::DRIVER, 5, 4), 52, 1, 800, 6000, 1);
Head head(xStepper, yStepper, HEAD_MEM_ADDR);

#endif