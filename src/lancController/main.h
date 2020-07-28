#include <Arduino.h>
#include "lancHandler.cpp"
#include "../commandStructure.h"

#define SOFTWARE_VERSION_MAJOR 0
#define SOFTWARE_VERSION_MINOR 1

LancController lancController(3, 4);

void setup() {
  Serial.begin(115200);
  Serial.println("Kardinia Jib Controller 2020 - Lanc Controller");
  Serial.println(String("Version:") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR);
  Serial.println(String("Last Compile Date: ") + __DATE__);
  Serial.println("");
}

void loop() {
    //lancController.sendCommand(LancController::CameraCommand::zoomWide04);
    Serial.println(millis());
    lancController.sendCommand(LancController::CameraCommand::zoomTele04);
    Serial.println(millis());
  while(true);
    // delay(1000);
    // Serial.println("WIDE");
    // for(int i =0; i < 100; i++){lancController.sendCommand(LancController::CameraCommand::zoomWide04);}
    // delay(1000);
    //while(true);
}