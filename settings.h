/**
    Settings File
    Responsible for defining the settings of the system
**/

#ifndef SETTINGS_FILE
#define SETTINGS_FILE

#include "joystick.h"
#include "head.h"

#define MEMORY_LEAD_0 0x59
#define MEMORY_LEAD_1 0x45
#define MEMORY_LEAD_2 0x45
#define MEMORY_LEAD_3 0x54
#define RIGHTJOY_MEM_ADDR 5
#define HEAD_MEM_ADDR 20

//Hardware
#define SPEED_POT_PIN A1
#define ACCEL_POT_PIN A0

//JoyStick Settings
JoyStick rightJoyStick(A5, A6, A7, 521, 519, 519, 0, 0, 0, 160, 155, RIGHTJOY_MEM_ADDR);

//Head Settings
//Stepper(AccelStepper stepper(int stepPin, int directionPin), int limitPin, boolean invert, int maxPosition, int homePosition, int maxSpeed, int defaultAcceleration, int invertLimit=0) {
Stepper xStepper(AccelStepper(AccelStepper::DRIVER, 3, 2), 50, 1, 6000, 2000, 1400, 10000);
Stepper yStepper(AccelStepper(AccelStepper::DRIVER, 5, 4), 52, 1, 600, 2000, 1400, 10000);
Head head(xStepper, yStepper, HEAD_MEM_ADDR);

#endif