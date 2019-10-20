
/**
 * Kardinia Jib Project Version 2
**/

#include <Arduino.h>
#include "settings.h"

#include <EEPROMex.h>

#define SOFTWARE_VERSION_MAJOR 2
#define SOFTWARE_VERSION_MINOR 2

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
    leftLCD.initalize();
    rightLCD.initalize();
    leftLCD.showStartup(String("Version: ") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR + String("\n(") + __DATE__ + String(")"));

    //Check if the EEPROM has a valid start of memory if not clear it
    Serial.print("[SETUP] Check memory... ");
    if(EEPROM.readByte(0) != MEMORY_LEAD_0 || EEPROM.readByte(1) != MEMORY_LEAD_1 || EEPROM.readByte(2) != MEMORY_LEAD_2 || EEPROM.readByte(3) != MEMORY_LEAD_3 ||
        EEPROM.readByte(END_OF_MEMORY) != MEMORY_END_0 || EEPROM.readByte(END_OF_MEMORY + 1) != MEMORY_END_1) {
          //Memory is not valid we need to reset it
          Serial.println(" Not Valid. Resetting memory");
          resetMemory();    
     }
     else {Serial.println(" Valid");}

    //Read the parameters in memory and set them
    Serial.println("[SETUP] Read settings from memory");

    if(!rightJoyStick.readSettingsFromMemory() || !controlPanel.readSettingsFromMemory() || !head.readSettingsFromMemory()) {
      Serial.println("[ERROR] Critical error: failed to read memory from one or more objects");
      Serial.println("Process cannot continue.\n\nEEPROM has been reset please disconnect power and recalibate the system");
      resetMemory();
      leftLCD.showError("Invalid MEM", "Please reset and", "recalibate via serial");
      while(true){
        digitalWrite(DEBUG_LED, HIGH);
        delay(100);
        digitalWrite(DEBUG_LED, LOW);
        delay(100);
      }
    }

    //If the joystick is not calibrated calibrate it
    if(!rightJoyStick.checkSettings()) {
      Serial.println("[ERROR] The right joystick has invalid settings and will need to be recalibrated. Starting calibration utility...");
      leftLCD.showError("Calibration ER", "Please connect", "serial and calibrate");
      rightJoyStick.calibrate();
    }

    //If the control panel is not calibrated calibrate it
    if(!controlPanel.checkSettings()) {
      Serial.println("[ERROR] Control panel has invalid settings and will need to be recalibrated. Starting calibration utility...");
      leftLCD.showError("Calibration ER", "Please connect", "serial and calibrate");
      controlPanel.calibrate();
    }

    //Check if the read settings were correct if not they will need to be calibrated
    if(!head.checkSettings()) {
      Serial.println("[ERROR] Head has invalid settings and will need to be recalibrated. Starting calibration utility...");
      leftLCD.showError("Calibration ER", "Please connect", "serial and calibrate");
      head.calibrate(A1);
    }

    //Begin homing of the head
    head.home();
    Serial.println("[SETUP] Complete");
    leftLCD.clear();
    rightLCD.clear();
}

//Main loop
void loop() {
    blinkDebugLed();
    head.moveXY(rightJoyStick.getPercentage(JoyStick::Axis::X), rightJoyStick.getPercentage(JoyStick::Axis::Y), controlPanel.getPotPercentage(ControlPanel::Pot::Right), controlPanel.getPotPercentage(ControlPanel::Pot::Left));
    if(!head.run() && !rightJoyStick.isActive()) {
      leftLCD.showValue("Acceleration", (String)(int)controlPanel.getPotPercentage(ControlPanel::Pot::Left) + "%");
      rightLCD.showValue("Speed", (String)(int)controlPanel.getPotPercentage(ControlPanel::Pot::Right) + "%");
    }
}

//Heartbeat
unsigned long heartBeat = 0;
void blinkDebugLed() {
  if(millis() - heartBeat >= 500) {
    heartBeat = millis();
    digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));
  }
}

//Reset the memory pool to defaults
void resetMemory() {
  for(int i = 0; i < END_OF_MEMORY; i++) {
    EEPROM.write(i, 255);
  }

  //Write the lead and end of memory for check
  EEPROM.write(0, MEMORY_LEAD_0);
  EEPROM.write(1, MEMORY_LEAD_1);
  EEPROM.write(2, MEMORY_LEAD_2);
  EEPROM.write(3, MEMORY_LEAD_3);
  EEPROM.write(END_OF_MEMORY, MEMORY_END_0);
  EEPROM.write(END_OF_MEMORY + 1, MEMORY_END_1);
}

//Print the memory to the serial monitor for debug
void printMemory() {
  for(int i = 0; i < END_OF_MEMORY + 2; i++) {
    Serial.print(EEPROM.read(i));
    if((i % 10 == 0 && i != 0) || i == END_OF_MEMORY - 1){Serial.println("");}else{Serial.print(",");}
  }
}
