/*
Kardinia Jib
By Kardinia Church 2020

headHandler.h
Main entry point for the head
*/

#define SOFTWARE_VERSION_MAJOR 0
#define SOFTWARE_VERSION_MINOR 1

#include <Arduino.h>
#include "settings.h"

void setup(){
    Serial.begin(115200);
    Serial.println("Kardinia Jib Controller 2020 - Head Firmware");
    Serial.println(String("Version:") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR);
    Serial.println(String("Last Compile Date: ") + __DATE__);

    Serial.println("Setup stepper motors");
    panStepper.setup(HEAD_MEM_ADDR);
    tiltStepper.setup(HEAD_MEM_ADDR + STEPPER_MEM_ALLOC);
}
void loop(){}