#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WebSocketServer.h>
#include "lancHandler.cpp"
#include "../SerialCommunication.h"

#define SOFTWARE_VERSION_MAJOR 0
#define SOFTWARE_VERSION_MINOR 1

LancController lancController(3, 4);
SoftwareSerial Serial1(8, 9); //RX TX
SerialCommunication serialCommunication(&Serial1);

byte mac[]{ 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 46, 180);
constexpr uint16_t port = 3000;
WebSocketServer wss(port);

void(* resetFunc) (void) = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Kardinia Jib Controller 2020 - Lanc Controller");
  Serial.println(String("Version:") + SOFTWARE_VERSION_MAJOR + String(".") + SOFTWARE_VERSION_MINOR);
  Serial.println(String("Last Compile Date: ") + __DATE__);
  Serial.println("");
  Serial.print("Connecting to Ethernet..");
  if(Ethernet.begin(mac) == 0) {
      Serial.println(" Failed to connect. Rebooting");
      delay(1000);
      resetFunc();
    }
    else {
      Serial.print(" Connected IP: ");
      Serial.print(Ethernet.localIP());
      Serial.print(", Listening on port ");
    }

  Serial1.begin(38400);
}

void loop() {
  // serialCommunication.sendAbsoluteMove(100, 200, 1.0, 2.0);
  //   delay(1000);
  // serialCommunication.sendRelativeMove(100, 200, 1.0, 2.0);
  //   delay(1000);
  // serialCommunication.sendSpeedMove(50.0, -50.0, 1.0);
  //   delay(1000);
  // serialCommunication.sendHeadReset();
  //   delay(1000);

  //serialCommunication.sendLancCommand(SerialCommunication::LancCommand::ZoomIn, SerialCommunication::LancZoomValue::Speed3);
  // serialCommunication.sendHeadCommand(SerialCommunication::HeadCommand::RelMove, 2, new (int[2]){0, 1});
  // int arrrrr[] = {255, 4, 5,6};
  // generatePacket(Serial1, 0, 1, 2, sizeof(arrrrr) / sizeof(arrrrr[0]), arrrrr);
  // int arrrrr[] = {255, 4, 5,6};

  // int ret[MAXIMUM_SERIAL_BUFFER_LENGTH] = generatePacket(0, 1, 2);//, sizeof(arrrrr) / sizeof(arrrrr[0]), arrrrr);
  
  // for(int i = 0; i < sizeof(ret) / sizeof(ret[0]); i++) {
  //   Serial.println(ret[i]);
  // }

}


// void loop() {
//   sendLancCommand(LancCommand::ZoomIn, ZoomValue::Speed1, HEAD_ADDR);
//   Serial.println("SEND");
//   delay(1000);


//   //   //lancController.sendCommand(LancController::CameraCommand::zoomWide04);
//   //   Serial.println(millis());
//   //   lancController.sendCommand(LancController::CameraCommand::zoomTele04);
//   //   Serial.println(millis());
//   // while(true);
//   //   // delay(1000);
//   //   // Serial.println("WIDE");
//   //   // for(int i =0; i < 100; i++){lancController.sendCommand(LancController::CameraCommand::zoomWide04);}
//   //   // delay(1000);
//   //   //while(true);
// }
