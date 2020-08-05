#include <Arduino.h>
#include <SoftwareSerial.h>
#include "lancHandler.cpp"
#include "../SerialCommunication.h"

#include <UIPEthernet.h>

#define SOFTWARE_VERSION_MAJOR 0
#define SOFTWARE_VERSION_MINOR 1

#define INCOMING_PORT 23489
#define OUTGOING_PORT 21342
#define PACKET_SIZE 64

byte mac[] = {0x1C, 0x39, 0x47, 0x10, 0x0A, 0x8F};
EthernetUDP udp;
String udpPassword = "tricaster";
bool foundServer = false;
IPAddress serverIP(192, 168, 0, 5);

IPAddress ip(10, 0, 0, 5);
IPAddress gateway(10, 0, 0, 138);
IPAddress sub(255, 255, 255, 0);


LancController lancController(3, 4);
SoftwareSerial Serial1(8, 9); //RX TX
SerialCommunication serialCommunication(&Serial1);

void(* resetFunc) (void) = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Kardinia Jib Controller 2020 - Lanc Controller");
  Serial.println(String("Version:") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR);
  Serial.println(String("Last Compile Date: ") + __DATE__);
  Serial.println("");
  Serial.print("Connecting to Ethernet..");

Ethernet.begin(mac, ip, gateway, sub);

  // if(Ethernet.begin(mac) == 0) {
  //     Serial.println(" Failed to connect. Rebooting.. ");
  //     delay(1000);
  //     resetFunc();
  //   }
  //   else {
  //     Serial.print(" Connected IP: ");
  //     Serial.print(Ethernet.localIP());
  //     udp.begin(INCOMING_PORT);
  //     Serial.print(", Listening on port ");
  //     Serial.println(INCOMING_PORT);
  //   }
udp.begin(INCOMING_PORT);
Serial.print(" Connected IP: ");
     Serial.print(Ethernet.localIP());
  Serial1.begin(38400);
  serialCommunication.sendStatus(SerialCommunication::StatusValue::Ready);
}

//Send out a network command to the server for updates
void sendMessageToServer(String message) {
    udp.beginPacket(udp.remoteIP(), OUTGOING_PORT);
    udp.print("KJIB");
    udp.endPacket();
    udp.stop();
    udp.begin(INCOMING_PORT);
}

void processLancIncoming(SerialCommunication::LancCommand command, int value, int dataSize = 0, int *data = nullptr) {
      //Process the lanc commands locally and output to the lanc
      switch(command) {
        Serial.print("Sending LANC command: ZoomIn");
        case SerialCommunication::LancCommand::ZoomIn: {
          int cmd = LancController::CameraCommand::zoomTele01 + value;
          if(cmd < LancController::CameraCommand::zoomTele01){cmd = LancController::CameraCommand::zoomTele01;}
          if(cmd > LancController::CameraCommand::zoomTele08){cmd = LancController::CameraCommand::zoomTele08;}
          lancController.sendCommand(cmd);
          Serial.println(" Done");
          break;
        }
        case SerialCommunication::LancCommand::ZoomOut: {
          break;
        }
        case SerialCommunication::LancCommand::Rec: {
          break;
        }
        case SerialCommunication::LancCommand::Focus: {
          break;
        }
        case SerialCommunication::LancCommand::WhiteBalance: {
          break;
        }
        case SerialCommunication::LancCommand::Iris: {
          break;
        }
        case SerialCommunication::LancCommand::Shutter: {
          break;
        }
      }
}

//Process the network incomers
/*
Expects a packet as follows

KJIB [PASSWORD] [COMMANDS TO PASS (CHECK JIB DOCUMENTATION)]
*/
char packetBuffer[PACKET_SIZE];
void processNetworkIncoming() {
  int incoming = udp.parsePacket();

  //Got packet?
  if (incoming) {
    memset(packetBuffer, 0, sizeof(packetBuffer));
    udp.read(packetBuffer, PACKET_SIZE);
    IPAddress remoteIP = udp.remoteIP();

    //Check KJIB flag
    if(packetBuffer[0] != 0x4B){return false;}
    if(packetBuffer[1] != 0x4A){return false;}
    if(packetBuffer[2] != 0x49){return false;}
    if(packetBuffer[3] != 0x42){return false;}

    serverIP = udp.remoteIP();
    foundServer = true;

    //If the incoming length is 4 the server is asking where we are
    if(incoming == 4){
      Serial.println("Replied to server ping request");
      sendMessageToServer("KJIB");
    }

    //Check password
    for(int i = 0; i < udpPassword.length(); i++) {
      if(packetBuffer[i + 4] != udpPassword[i]) {
        //If the password is incorrect
        return false;
      }
    }

    SerialCommunication::Type type = packetBuffer[4 + udpPassword.length() + 0];
    int command = packetBuffer[4 + udpPassword.length() + 1];
    int value = packetBuffer[4 + udpPassword.length() + 2];
    int dataSize = packetBuffer[4 + udpPassword.length() + 3];
    int data[dataSize];

    for(int i = 0; i < dataSize; i++) {
      data[i] = packetBuffer[4 + udpPassword.length() + 4 + i];
    }

    //Process lanc commands here and pass head commands to the control panel
    switch(type) {
      case SerialCommunication::Type::Head: {
        Serial.println("Sent head command to the control panel");
        serialCommunication.sendHeadCommand(command, dataSize, data);
        break;
      }
      case SerialCommunication::Type::Lanc: {
        processLancIncoming(command, value, dataSize, data);
        break;
      }
    }

    // Serial.println(type);
    // Serial.println(command);
    // Serial.println(value);
    // Serial.println(dataSize);

    // for(int i = 0; i < dataSize; i++) {
    //   Serial.println(data[i]);
    // }

    // Serial.println("END");
    

    //Fix as it will stop listening when the server dies for some reason
    udp.stop();
    udp.begin(INCOMING_PORT);
  }
}

int lastEthernetState = 0;
void loop() {
  processNetworkIncoming();
  Serial.println(".");

  //Send network updates to the main cpu
  if(lastEthernetState != Ethernet.linkStatus()) {
    lastEthernetState = Ethernet.linkStatus();
    switch(Ethernet.linkStatus()) {
      case LinkON: {
        Serial.println("Network connected");
        serialCommunication.sendStatus(SerialCommunication::StatusValue::NetworkConnected);
        break;
      }
      case LinkOFF: {
        Serial.println("Network disconnected");
        serialCommunication.sendStatus(SerialCommunication::StatusValue::NetworkDisconnected);
        break;
      }
      case Unknown: {
        Serial.println("Network disconnected");
        serialCommunication.sendStatus(SerialCommunication::StatusValue::NetworkDisconnected);
        break;
      }
    }
  }

  Ethernet.maintain();
}