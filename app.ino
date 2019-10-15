
/**
 * Kardinia Jib Project Version 2
**/

#include <Arduino.h>
#include "joystick.h"
#include "head.h"

#define SOFTWARE_VERSION_MAJOR 2
#define SOFTWARE_VERSION_MINOR 0
#define DEBUG_LED 13
#define IGNORE_HOME 0


//Left pot = A0
//Right pot = A1
//Buttons 22 ->

//Joy x = A5
//Joy y = A6
//Joy z = A7

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
    joystickSetup();
    headSetup();
    Serial.println("[SETUP] Complete");
    pinMode(A0, INPUT);
}

//Main loop
void loop() {
    blinkDebugLed();
    //joystickDebug();
    headLoop();
    moveX(joystickXPercentage());
    moveY(joystickYPercentage());

}

//Heartbeat
unsigned long heartBeat = 0;
void blinkDebugLed() {
  if(millis() - heartBeat >= 500) {
    heartBeat = millis();
    digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));
  }
}