#include <Arduino.h>
///#include <SoftwareSerial.h>
#include "AltSoftSerial/AltSoftSerial.h"
#include "LibLanc/LibLanc.h"

#define SOFTWARE_VERSION_MAJOR 0
#define SOFTWARE_VERSION_MINOR 1
#define DEBUG_LED 13
Lanc lanc(3, 4);
//SoftwareSerial Serial1(8, 9); //RX TX
AltSoftSerial Serial1;

#include "../globals.h"

void(* resetFunc) (void) = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Kardinia Jib Controller 2020 - Lanc Controller");
  Serial.println(String("Version:") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR);
  Serial.println(String("Last Compile Date:` ") + __DATE__);
  Serial.println("");
  Serial1.begin(38400);
  Serial1.println("Lanc controller ready!");
  lanc.begin();
}

//Process the incoming serial data. Returns true when data was received
CommandType type;
int command = 0;
int value = 0;
int dataSize = 0;
int data[64];
bool processSerialCommunication() {
    int i = 0;
    if(Serial1.available()) {
        delay(40); //Wait for the buffer to fill
        while(Serial1.available()) {
            char current = Serial1.read();
            switch(i){
              case 0: {type = (int)current; break;}
              case 1: {command = (int)current; break;}
              case 2: {value = (int)current; break;}
              case 3: {dataSize = (int)current; break;}
            }

            //Read in the data
            if(i > 3 && i < dataSize) {
              data[i - 4] = (int)current;
            }

            i++;
        }
        return true;
    }
    return false;
}

//Send a serial message to both onboard serial and external serial for debugging
void sendSerial(String str) {
  Serial.println(str); 
}

//Heartbeat
unsigned long heartBeat = 0;
void blinkDebugLed() {
  if(millis() - heartBeat >= 500) {
    heartBeat = millis();
    digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));
  }
}

void loop() {
  blinkDebugLed();
  if(processSerialCommunication()) {
    switch(type) {
      case CommandType::Lanc: {
        switch(command) {
          case LancCommand::Zoom: {
            sendSerial("Sending LANC command: Zoom " + (String)value);
            lanc.Zoom(value);
            break;
          }
          case LancCommand::Focus: {
            sendSerial("Sending LANC command: Focus " + (String)value);
            lanc.Focus(value);
            break;
          }
          case LancCommand::AutoFocus: {
            sendSerial("Sending LANC command: Auto Focus " + (String)value);
            lanc.AutoFocus();
            break;
          }
        }
        break;
      }
      case CommandType::Control: {
        switch(command) {
          case ControlCommand::Reboot: {
            resetFunc();
            break;
          }
        }
        break;
      }
    }
  }

  lanc.loop();
}