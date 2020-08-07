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
    Serial.begin(115200);
    Serial1.begin(38400);
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

    // if(!rightJoyStick.checkSettings() || !controlPanel.checkSettings()) {
    //   leftLCD.clear();
    //   rightLCD.clear();
    //   leftLCD.showError("Calibration", "Calibration", "is required");
    //   rightLCD.showText("Press to begin", "Begin Cal >");

    //   while(true) {
    //     bool leave = false;
    //     for(int i = 0; i < TOTAL_COLS; i++) {
    //       if(controlPanel.isButtonsPressed().buttonStates[3][i]){leave = true;}
    //     }
    //     if(leave){break;}
    //   }

    //   //If the joystick is not calibrated calibrate it
    //   if(!rightJoyStick.checkSettings()) {
    //     Serial.println("\nThe right joystick has invalid settings and will need to be recalibrated. Starting calibration utility...");
    //     rightJoyStick.calibrate(leftLCD, rightLCD);
    //   }

    //   //If the control panel is not calibrated calibrate it
    //   if(!controlPanel.checkSettings()) {
    //     Serial.println("\nControl panel has invalid settings and will need to be recalibrated. Starting calibration utility...");
    //     controlPanel.calibrate(leftLCD, rightLCD);
    //   }
    // } 

    //Connect to network
    Serial.print("Attempting connection to network...");
    if(networkHandler.begin()) {
      Serial.print(" Success! IPAddress: ");
      Serial.print(networkHandler.localIP());
      Serial.print(":");
      Serial.println(networkHandler.incomingPort());
    }
    else {
      Serial.println(" Failed");
    }
   
    //Begin homing of the head
    rightLCD.clear();
    if(head.reset() != Stepper::HomeStatus::Complete){
      //Failed homing
      _errorText += "Home Failed!";
    }

    if(_errorText == ""){Serial.println("Setup Complete");}
    else {Serial.println("Setup Error - " + _errorText);}

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

//Send a command out to the serial
void sendDataToSerial(CommandType type, int command, int value, int dataSize = 0, int *data = nullptr) {
  Serial1.write(type);
  Serial1.write(command);
  Serial1.write(value);
  Serial1.write(dataSize);
  for(int i = 0; i < networkHandler.dataSize(); i++) {
    Serial1.write(data[i]);
  }
}

//Send a zoom to the lanc -8 to 8
void sendZoom(int speed) {
  if(speed < -8){speed = -8;}
  if(speed > 8){speed = 8;}
  sendDataToSerial(CommandType::Lanc, LancCommand::Zoom, speed);
}

//Send a focus to the lanc 0 - 1
void sendFocus(int direction) {
  if(direction < 0){direction = 0;}
  if(direction > 1){direction = 1;}
  sendDataToSerial(CommandType::Lanc, LancCommand::Focus, direction);
}

//Send a auto focus to lanc
void sendAutoFocus() {
  sendDataToSerial(CommandType::Lanc, LancCommand::AutoFocus, 0);
}

//Pass the network poacket over serial to the lanc device for processing
void passNetworkDataToSerial() {
  sendDataToSerial(networkHandler.type(), networkHandler.command(), networkHandler.value(), networkHandler.dataSize(), networkHandler.data());
}

int16_t getInt16(int *buffer, int index) {
  return static_cast<unsigned>(buffer[index]) << 8 | static_cast<unsigned>(buffer[index + 1]);
}

int32_t getInt32(int *buffer, int index) {
  union ArrayToInteger {
    byte array[4];
    int32_t integer;
  };
  ArrayToInteger conv;
  conv.array[0] = buffer[index];
  conv.array[1] = buffer[index + 1];
  conv.array[2] = buffer[index + 2];
  conv.array[3] = buffer[index + 3];

  return conv.integer;
}

//Main loop
unsigned long test = 0;
void loop() {
    blinkDebugLed();

    //Process the incoming network command if there is one
    if(networkHandler.process()) {
      switch(networkHandler.type()) {
        case CommandType::Movement: {
          switch(networkHandler.command()) {
            case MovementCommand::RelMove: {
              if(networkHandler.dataSize() != 0) {
                int32_t x = getInt32(networkHandler.data(), 0);
                int32_t y = getInt32(networkHandler.data(), 4);
                float speed = (float)getInt16(networkHandler.data(), 8) / 327.0;
                float acceleration = (float)getInt16(networkHandler.data(), 10) / 327.0;
                Serial.print("Moving rel to X: ");Serial.print(x); Serial.print(" Y:");Serial.print(y); Serial.print(" Speed:"); Serial.print(speed); Serial.print("% Accel:"); Serial.print(acceleration); Serial.println("%");
                head.moveRelative(x, y, speed, acceleration);
              }

              break;
            }
            case MovementCommand::AbsMove: {
              if(networkHandler.dataSize() != 0) {
                int32_t x = getInt32(networkHandler.data(), 0);
                int32_t y = getInt32(networkHandler.data(), 4);
                float speed = (float)getInt16(networkHandler.data(), 8) / 327.0;
                float acceleration = (float)getInt16(networkHandler.data(), 10) / 327.0;
                Serial.print("Moving abs to X: ");Serial.print(x); Serial.print(" Y:");Serial.print(y); Serial.print(" Speed:"); Serial.print(speed); Serial.print("% Accel:"); Serial.print(acceleration); Serial.println("%");
                head.moveToXY(x, y, speed, acceleration);
              }
              break;
            }
            case MovementCommand::MoveSpeed: {
              if(networkHandler.dataSize() != 0) {
                float speedX = (float)getInt16(networkHandler.data(), 0) / 327.0;
                float speedY = (float)getInt16(networkHandler.data(), 2) / 327.0;
                float acceleration = (float)getInt16(networkHandler.data(), 4) / 327.0;
                Serial.print("Moving speed at X: ");Serial.print(speedX); Serial.print("% Y:");Serial.print(speedY); Serial.print("% Accel:"); Serial.print(acceleration); Serial.println("%");
                head.moveXY(speedX, speedY, acceleration);
              }
              break;
            }
            case MovementCommand::Stop: {
              float acceleration = (float)getInt16(networkHandler.data(), 0) / 327.0;
              Serial.print("Stopping head at Accel:");Serial.print(acceleration); Serial.println("%");
              head.stop(acceleration);
              break;
            }
          }
          break;
        }
        case CommandType::Lanc: {
          switch(networkHandler.command()) {
            case LancCommand::Zoom: {
              passNetworkDataToSerial();
              break;
            }
            case LancCommand::Focus: {
              passNetworkDataToSerial();
              break;
            }
            case LancCommand::AutoFocus: {
              passNetworkDataToSerial();

              break;
            }
          }
          break;
        }
        case CommandType::Control: {
          switch(networkHandler.command()) {
            case ControlCommand::Reboot: {
              Serial.println("Network request to reboot. Will reboot in 5 seconds");
              leftLCD.showText("Reboot", "", "Will reboot in 5 seconds", "Requested from network");
              rightLCD.showText("Reboot", "", "Will reboot in 5 seconds", "Requested from network");
              networkHandler.sendCommand(CommandType::Control, ControlCommand::Reboot, 0);
              passNetworkDataToSerial();
              delay(5000);
              resetFunc();
              break;
            }
          }
          break;
        }
      }
    }

    //Process incoming serial from the lanc control
    if(Serial1.available()) {
        while(Serial1.available()) {
          Serial.write(Serial1.read());
        }
    }

    if(head.isMoving()) {
      while(head.isMoving()) {
        if(controlPanel.isStopButtonPressed()) {
          //Stop
          head.stop(20000.0);
        }

        if(!head.movingToPosition()) {
          //processJoyStick();
        }

        if(head.isMovingRelative()) {
          head.setMaxSpeed(controlPanel.getPotPercentage(ControlPanel::Pot::Right));
        }

        head.run();
      }
    }

    // //Update the LCDs
    leftLCD.setTextToShow("Focus", (String)(int)controlPanel.getPotPercentage(ControlPanel::Pot::Left) + "%", "NOT USED", "NOT USED");
    rightLCD.setTextToShow("Speed", (String)(int)controlPanel.getPotPercentage(ControlPanel::Pot::Right) + "%", "", _errorText);
    //leftLCD.update();
    rightLCD.update();

    // //processJoyStick();
    checkButtons();
    head.run();
}