/*
Settings file responsible for defining the settings for the system
By Kardinia Church 2020
*/

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

#define HEAD_MEM_ADDR 4

#include <EEPROMex.h>
#include "stepper.h"

Stepper panStepper(3, 2, 50, false, 2000, 100, true);
Stepper tiltStepper(5, 4, 52, true, 2000, 100, true);

#endif