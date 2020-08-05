/**
 * Kardinia Jib Project Version 2
**/

#include <Arduino.h>
#include "settings.h"

#include <EEPROMex.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SOFTWARE_VERSION_MAJOR 3
#define SOFTWARE_VERSION_MINOR 0

String _errorText = "";

//Heartbeat
unsigned long heartBeat = 0;
void blinkDebugLed() {
  if(millis() - heartBeat >= 500) {
    heartBeat = millis();
    digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));
  }
}

//Show important information on the LCDs
void showDebugLCD() {
    String leftLine1 = "LPOT " + (String)(int)controlPanel.getPotPercentage(ControlPanel::Pot::Left);
    String leftLine2 = "";
    String leftLine3 = "";
    String leftLine4 = "";

    String rightLine1 = "RPOT " + (String)(int)controlPanel.getPotPercentage(ControlPanel::Pot::Right);
    String rightLine2 = "";
    String rightLine3 = (String)rightJoyStick.getPercentage(JoyStick::Axis::X) + "," + (String)rightJoyStick.getPercentage(JoyStick::Axis::Y) + "," + (String)rightJoyStick.getPercentage(JoyStick::Axis::Z);
    String rightLine4 = "";
    
    for(int i = 0; i < TOTAL_ROWS / 2; i++) {
      for(int j = 0; j < TOTAL_COLS; j++) {
        leftLine4 += (String)controlPanel.isButtonsPressed().buttonStates[i][j] + " ";
        rightLine4 += (String)controlPanel.isButtonsPressed().buttonStates[i + 2][j] + " ";
      }
      leftLine4 += ",";
      rightLine4 += ",";
    }

    leftLCD.showText(leftLine1, leftLine2, leftLine3, leftLine4);
    rightLCD.showText(rightLine1, rightLine2, rightLine3, rightLine4);
}

//Reset the memory pool to defaults
void resetMemory() {
  // for(int i = 0; i < END_OF_MEMORY; i++) {
  //   EEPROM.write(i, 255);
  // }

  // //Write the lead and end of memory for check
  // EEPROM.write(0, MEMORY_LEAD_0);
  // EEPROM.write(1, MEMORY_LEAD_1);
  // EEPROM.write(2, MEMORY_LEAD_2);
  // EEPROM.write(3, MEMORY_LEAD_3);
  // EEPROM.write(END_OF_MEMORY, MEMORY_END_0);
  // EEPROM.write(END_OF_MEMORY + 1, MEMORY_END_1);
}

//Print the memory to the serial monitor for debug
void printMemory() {
  for(int i = 0; i < END_OF_MEMORY + 2; i++) {
    Serial.print(EEPROM.read(i));
    if((i % 10 == 0 && i != 0) || i == END_OF_MEMORY - 1){Serial.println("");}else{Serial.print(",");}
  }
}

void(* resetFunc) (void) = 0; 

//Begin setup
void setup() {
    Serial1.begin(38400);
    Serial1.println("START");
    Serial.begin(115200);
    Serial.println("Kardinia Jib Controller 2019");
    Serial.println(String("Version:") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR);
    Serial.println(String("Last Compile Date: ") + __DATE__);
    Serial.println("");
    Serial.println("Send serial to begin calibration!");
    Serial.println("");
    leftLCD.initalize();
    rightLCD.initalize();
    leftLCD.showStartup(String("Version: ") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR + String("\n(") + __DATE__ + String(")"));
    rightLCD.showText("Press for:", "< Cal , Test >", "", "");
    long timeout = millis();
    bool calibrate = false;
    while(timeout + 3000 > millis()) {
      for(int i = 0; i < 3; i++) {
        if(controlPanel.isButtonsPressed().buttonStates[2][i]) {
          resetMemory();
          rightLCD.showText("OK", "Calibration");
        }
        else if(controlPanel.isButtonsPressed().buttonStates[3][i]) {
          while(true) {
            showDebugLCD();
          }
        }
      }
      while (Serial.available()) {
        calibrate = true;
        break;
      }
    }
    if(calibrate){resetMemory ();}
    rightLCD.clear();

    pinMode(DEBUG_LED, OUTPUT);
    digitalWrite(13, HIGH);

    //Check if the EEPROM has a valid start of memory if not clear it
    Serial.print("Check memory... ");
    if(EEPROM.readByte(0) != MEMORY_LEAD_0 || EEPROM.readByte(1) != MEMORY_LEAD_1 || EEPROM.readByte(2) != MEMORY_LEAD_2 || EEPROM.readByte(3) != MEMORY_LEAD_3 ||
        EEPROM.readByte(END_OF_MEMORY) != MEMORY_END_0 || EEPROM.readByte(END_OF_MEMORY + 1) != MEMORY_END_1) {
          //Memory is not valid we need to reset it
          Serial.println(" Not Valid. Resetting memory");
          resetMemory();    
     }
     else {Serial.println(" Valid");}

    //Read the parameters in memory and set them
    Serial.println("Read settings from memory");

    if(!rightJoyStick.readSettingsFromMemory() || !controlPanel.readSettingsFromMemory()) {
      Serial.println("Critical error: failed to read memory from one or more objects");
      Serial.println("Process cannot continue.\n\nEEPROM has been reset please disconnect power and recalibate the system");
      resetMemory();
      leftLCD.showError("Invalid MEM", "Please reset and", "recalibate");
      while(true){
        digitalWrite(DEBUG_LED, HIGH);
        delay(100);
        digitalWrite(DEBUG_LED, LOW);
        delay(100);
      }
    }

    if(!rightJoyStick.checkSettings() || !controlPanel.checkSettings()) {
      leftLCD.clear();
      rightLCD.clear();
      leftLCD.showError("Calibration", "Calibration", "is required");
      rightLCD.showText("Press to begin", "Begin Cal >");

      while(true) {
        bool leave = false;
        for(int i = 0; i < TOTAL_COLS; i++) {
          if(controlPanel.isButtonsPressed().buttonStates[3][i]){leave = true;}
        }
        if(leave){break;}
      }

      //If the joystick is not calibrated calibrate it
      if(!rightJoyStick.checkSettings()) {
        Serial.println("\nThe right joystick has invalid settings and will need to be recalibrated. Starting calibration utility...");
        rightJoyStick.calibrate(leftLCD, rightLCD);
      }

      //If the control panel is not calibrated calibrate it
      if(!controlPanel.checkSettings()) {
        Serial.println("\nControl panel has invalid settings and will need to be recalibrated. Starting calibration utility...");
        controlPanel.calibrate(leftLCD, rightLCD);
      }
    }

    //Begin homing of the head
    rightLCD.clear();
    if(head.reset() != Stepper::HomeStatus::Complete){
      //Failed homing
      _errorText = "Home Failed!";
    }

    if(_errorText == ""){Serial.println("Complete");}
    else {Serial.println("Failed - " + _errorText);}

    leftLCD.clear();
    rightLCD.clear();
}

//Check the buttons for speicific moves
void checkButtons() {
    if(controlPanel.isButtonsPressed().buttonStates[0][0]) {
      //Centre head
      head.goHome(100.0, 500.0);
    }
    if(controlPanel.isButtonsPressed().buttonStates[0][1]) {
      //Reboot
      leftLCD.showText("Reboot", "", "Are you sure?", "Keep holding");
      delay(5000);
      if(controlPanel.isButtonsPressed().buttonStates[0][1]) {
        resetFunc();
      }
      leftLCD.clear();
      leftLCD.setTextToShow("", "", "", "");
      leftLCD.update();
    }
    if(controlPanel.isButtonsPressed().buttonStates[2][0]) {
      //Pan left 45 degreesish
      head.moveRelative(1000000, 0, controlPanel.getPotPercentage(ControlPanel::Pot::Right), 20000.0);
    }
    if(controlPanel.isButtonsPressed().buttonStates[1][0]) {
      //Pan right 45 degreesish
      head.moveRelative(-1000000, 0, controlPanel.getPotPercentage(ControlPanel::Pot::Right), 20000.0);
    }
    if(controlPanel.isButtonsPressed().buttonStates[1][1]) {
      //Tilt up 45 degreesish
      head.moveRelative(0, 10000000, controlPanel.getPotPercentage(ControlPanel::Pot::Right), 20000.0);
    }
    if(controlPanel.isButtonsPressed().buttonStates[2][1]) {
      //Tilt down 45 degreesish
      head.moveRelative(0, -1000000, controlPanel.getPotPercentage(ControlPanel::Pot::Right), 20000.0);
    }
    if(controlPanel.isButtonsPressed().buttonStates[1][2]) {
      //Move both tilt and pan
      head.moveRelative(10000000, 10000000, controlPanel.getPotPercentage(ControlPanel::Pot::Right), 20000.0);
    }
    if(controlPanel.isButtonsPressed().buttonStates[2][2]) {
      //Move both tilt and pan
      head.moveRelative(-1000000, -1000000, controlPanel.getPotPercentage(ControlPanel::Pot::Right), 20000.0);
    }
}

void processJoyStick() {
  float acceleration = 100.0;
  float globalSpeed = controlPanel.getPotPercentage(ControlPanel::Pot::Right);
  float xSpeed = (rightJoyStick.getPercentage(JoyStick::Axis::X) / 100.0) * (globalSpeed / 100.0);
  float ySpeed = (rightJoyStick.getPercentage(JoyStick::Axis::Y) / 100.0) * (globalSpeed / 100.0);
  head.moveXY(xSpeed * 100, ySpeed * 100, 100.0);
}



//Main loop
void loop() {
    blinkDebugLed();

    //Handle serial commands when sent
    if(serialCommunication.processSerialCommunication()) {
      switch(serialCommunication.getSerialType()) {
        case SerialCommunication::Type::Head: {
          switch(serialCommunication.getSerialCommand()) {
            case SerialCommunication::HeadCommand::RelMove: {
              long x = serialCommunication.getLongValue(serialCommunication.getSerialData(), 0);
              long y = serialCommunication.getLongValue(serialCommunication.getSerialData(), 4);
              float speed = serialCommunication.getFloatValue(serialCommunication.getSerialData(), 8);
              float acceleration = serialCommunication.getFloatValue(serialCommunication.getSerialData(), 12);
              Serial.print("GOT: Rel move request X=");Serial.print(x); Serial.print(" Y="); Serial.print(y); Serial.print(" Speed="); Serial.print(speed); Serial.print(" Accel="); Serial.println(acceleration);

              head.moveRelative(x, y, speed, acceleration);
              break;
            }
            case SerialCommunication::HeadCommand::AbsMove: {
              long x = serialCommunication.getLongValue(serialCommunication.getSerialData(), 0);
              long y = serialCommunication.getLongValue(serialCommunication.getSerialData(), 4);
              float speed = serialCommunication.getFloatValue(serialCommunication.getSerialData(), 8);
              float acceleration = serialCommunication.getFloatValue(serialCommunication.getSerialData(), 12);
              Serial.print("GOT: Abs move request X=");Serial.print(x); Serial.print(" Y="); Serial.print(y); Serial.print(" Speed="); Serial.print(speed); Serial.print(" Accel="); Serial.println(acceleration);

              head.moveToXY(x, y, speed, acceleration);
              break;
            }
            case SerialCommunication::HeadCommand::MoveSpeed: {
              float x = serialCommunication.getFloatValue(serialCommunication.getSerialData(), 0);
              float y = serialCommunication.getFloatValue(serialCommunication.getSerialData(), 4);
              float acceleration = serialCommunication.getFloatValue(serialCommunication.getSerialData(), 8);
              Serial.print("GOT: Speed move request X=");Serial.print(x); Serial.print(" Y="); Serial.print(y); Serial.print(" Accel="); Serial.println(acceleration);

              head.moveXY(x, y, acceleration);
              break;
            }
            case SerialCommunication::HeadCommand::ResetHead: {
              Serial.println("GOT: Reset head request");
              leftLCD.showText("WARNING", "About to reset!", "Network request", "10 seconds to reset");
              leftLCD.showText("WARNING", "About to reset!", "Network request", "10 seconds to reset");
              delay(10000);
              resetFunc();
              break;
            }
          }
          break;
        }
      }
    }

    if(head.isMoving()) {
      while(head.isMoving()) {
        if(controlPanel.isStopButtonPressed()) {
          //Stop
          head.stop(20000.0);
        }

        if(!head.movingToPosition()) {
          processJoyStick();
        }

        if(head.isMovingRelative()) {
          head.setMaxSpeed(controlPanel.getPotPercentage(ControlPanel::Pot::Right));
        }

        //head.run();
      }
    }

    //Update the LCDs
    //leftLCD.setTextToShow("Focus", (String)(int)controlPanel.getPotPercentage(ControlPanel::Pot::Left) + "%", "NOT USED", "NOT USED");
    rightLCD.setTextToShow("Speed", (String)(int)controlPanel.getPotPercentage(ControlPanel::Pot::Right) + "%", "", _errorText);
    //leftLCD.update();
    rightLCD.update();

    processJoyStick();
    checkButtons();
    head.run();
}