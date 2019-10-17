/**
    Settings File
    Responsible for defining the settings of the system
**/

#ifndef SETTINGS_FILE
#define SETTINGS_FILE

#include "joystick.h"

#define MEMORY_LEAD_0 0x59
#define MEMORY_LEAD_1 0x45
#define MEMORY_LEAD_2 0x45
#define MEMORY_LEAD_3 0x54
#define RIGHTJOY_MEM_ADDR 5

//Hardware
#define SPEED_POT_PIN A1
#define ACCEL_POT_PIN A0

#define X_DIR_PIN 2
#define X_STEP_PIN 3
#define X_LIMIT_PIN 50
#define X_INVERT 1
#define Y_DIR_PIN 4
#define Y_STEP_PIN 5
#define Y_LIMIT_PIN 52
#define Y_INVERT 1

//Stepper settings
#define DEFAULT_ACCELERATION 10000
#define ACCEL_POT_MAX 1023
#define X_MAX_SPEED 1400
#define X_MAX_POSITION 6000
#define Y_MAX_SPEED 600
#define Y_MAX_POSITION 1000


//JoyStick
JoyStick rightJoyStick(A5, A6, A7, 521, 519, 519, 0, 0, 0, 160, 155, RIGHTJOY_MEM_ADDR);

#endif