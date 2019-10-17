
/**
 * Kardinia Jib Project Version 2
**/

#include <Arduino.h>
#include "settings.h"

#define SOFTWARE_VERSION_MAJOR 2
#define SOFTWARE_VERSION_MINOR 0
#define DEBUG_LED 13

//Left pot = A0
//Right pot = A1
//Buttons 22 ->

//Begin setup
void setup() {
    Serial.begin(115200);
    Serial.println("[INFO] Kardinia Jib Controller 2019");
    Serial.println(String("       Version:") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR);
    Serial.println(String("       Last Compile Date: ") + __DATE__);
    Serial.println("---------------------------------------");
    Serial.println("");
    Serial.println("[SETUP] Start");
    pinMode(DEBUG_LED, OUTPUT);
    digitalWrite(13, HIGH);

    //Check if the EEPROM has a valid start of memory if not clear it
    Serial.print("[SETUP] Check memory... ");
    if(EEPROM.read(0) != MEMORY_LEAD_0 || EEPROM.read(1) != MEMORY_LEAD_1 || EEPROM.read(2) != MEMORY_LEAD_2 || EEPROM.read(3) != MEMORY_LEAD_3) {
      Serial.println("Not set. Setting up memory");
      for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 255);
      }
      EEPROM.write(0, MEMORY_LEAD_0);
      EEPROM.write(1, MEMORY_LEAD_1);
      EEPROM.write(2, MEMORY_LEAD_2);
      EEPROM.write(3, MEMORY_LEAD_3);
    }
    else {Serial.println("Complete");}

    //Read the parameters in memory and set them
    Serial.println("[SETUP] Read settings from memory");
    rightJoyStick.readSettingsFromMemory();

    //Begin homing of the head
    head.home();



    Serial.println("[SETUP] Complete");
}

//Main loop
void loop() {
    blinkDebugLed();
    head.moveXY(rightJoyStick.getPercentage(JoyStick::Axis::X), rightJoyStick.getPercentage(JoyStick::Axis::Y));
    head.run();
}

//Heartbeat
unsigned long heartBeat = 0;
void blinkDebugLed() {
  if(millis() - heartBeat >= 500) {
    heartBeat = millis();
    digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));
  }
}