/*
Kardinia Jib By Kardinia Church 2020

networkHandler.cpp: Provides the processing for network communication


Command structure is as follows:

0       5       6       7           8       9                               10 - x
"KJIB"  ID      TYPE    COMMAND     VALUE   DATASIZE (0 if not required)    DATA
Note: Only KJIB is required for ping requests or replies

This device will send updates to the broadcast address (x.x.x.255)
This device will respond to requests over broadcast if the id is correct (id=255 is a broadcast)
This device will respond to requests by sending directly to the clients ip

TODO:
Add encription to the packet using the specficied password
*/

#ifndef NETWORK_HANDLER
#define NETWORK_HANDLER

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "../globals.h"

class NetworkHandler {
    private:
    bool _dhcpMode = false;
    byte *_mac;
    IPAddress _ip;
    IPAddress *_gateway;
    IPAddress *_subnet;
    int _incomingPort;
    int _outgoingPort;
    EthernetUDP _udp;
    char _packetBuffer[UDP_TX_PACKET_MAX_SIZE];
    String _password;
    int _id;

    CommandType _type;
    int _command;
    int _value;
    int _dataSize;
    int _data[64];

    public:
    NetworkHandler(int id, int incomingPort, int outgoingPort, String password, byte *mac) {
        _id = id;
        _dhcpMode = true;
        _incomingPort = incomingPort;
        _outgoingPort = outgoingPort;
        _password = password;
        _mac = mac;
    };
    NetworkHandler(int id, int incomingPort, int outgoingPort, String password, byte *mac, IPAddress ip) {
        _dhcpMode = false;
        _id = id;
        _incomingPort = incomingPort;
        _outgoingPort = outgoingPort;
        _password = password;
        _mac = mac;
        _ip = ip;
    };

    //Getters / Setters
    IPAddress localIP() {return Ethernet.localIP();}
    String localIPString() {return "" + (String)Ethernet.localIP()[0] + "." + (String)Ethernet.localIP()[1] + "." + (String)Ethernet.localIP()[2] + "." + (String)Ethernet.localIP()[3];}
    int incomingPort() {return _incomingPort;}
    int outgoingPort() {return _outgoingPort;}
    IPAddress getBroadcastAddress() {return IPAddress(Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], 255);}
    CommandType type() {return _type;}
    int command() {return _command;}
    int value() {return _value;}
    int dataSize() {return _dataSize;}
    int *data() {return _data;}

    //Attempt to connect to ethernet. Returns true if successful
    bool begin() {
        if(_dhcpMode) {
            if(Ethernet.begin(_mac) == 0){return false;}
        }
        else {
            Ethernet.begin(_mac, _ip);
        } 
        _udp.begin(_incomingPort);

        return true;
    }

    //Send a message  TODO ADD ENCRIPTION USING PASSOWRD
    void sendMessage(IPAddress goingAddress, int dataSize = 0, int *data = nullptr) {
        String lead = "KJIB";

        _udp.beginPacket(goingAddress, _outgoingPort);

        _udp.print(lead);
        _udp.write(_id);
        for(int i = 0; i < dataSize; i++) {
            _udp.write(data[i]);
        }
        _udp.endPacket();
    }

    //Send a broadcast message
    void sendMessage(int dataSize, int *data) {
        sendMessage(getBroadcastAddress(), dataSize, data);
    }

    //Are we connected to network?
    bool connected() {
        return Ethernet.linkStatus() != LinkOFF && Ethernet.localIP() != IPAddress(0,0,0,0);
    }

    //Ping the server to check for connection. This is blocking!
    bool serverConnected() {
        unsigned long timeout = millis() + 1000;
        while(timeout > millis()) {
            sendCommand(CommandType::Control, ControlCommand::Ping, 0);

            uint8_t packetSize = _udp.parsePacket();
            if(packetSize) {
                _udp.read(_packetBuffer, UDP_TX_PACKET_MAX_SIZE);

                //Check KJIB flag
                if(_packetBuffer[0] != 0x4B){return false;}
                if(_packetBuffer[1] != 0x4A){return false;}
                if(_packetBuffer[2] != 0x49){return false;}
                if(_packetBuffer[3] != 0x42){return false;}
                if(_packetBuffer[4] != _id){return false;}

                //Check command
                if(_packetBuffer[5] != CommandType::Control){return false;}
                if(_packetBuffer[6] != ControlCommand::Ping){return false;}
                if(_packetBuffer[7] != 0){return false;}
                if(_packetBuffer[8] != 0){return false;}

                return true;
            }
        }
    }

    //Send a command
    void sendCommand(IPAddress ip, CommandType type, int command, int value, int dataSize = 0, int *data = nullptr) {
        int send[4 + dataSize];
        send[0] = type;
        send[1] = command;
        send[2] = value;
        send[3] = dataSize;
        for(int i = 0; i < dataSize; i++){send[4 + i] = data[i];}
        sendMessage(ip, 4 + dataSize, send);
    }

    //Send a broadcast command
    void sendCommand(CommandType type, int command, int value, int dataSize = 0, int *data = nullptr) {
        sendCommand(getBroadcastAddress(), type, command, value, dataSize, data);
    }

    //Process the incoming data
    bool process() {
        uint8_t packetSize = _udp.parsePacket();
        if(packetSize) {
            IPAddress remote = _udp.remoteIP();
            _udp.read(_packetBuffer, UDP_TX_PACKET_MAX_SIZE);

            //Check KJIB flag
            if(_packetBuffer[0] != 0x4B){return false;}
            if(_packetBuffer[1] != 0x4A){return false;}
            if(_packetBuffer[2] != 0x49){return false;}
            if(_packetBuffer[3] != 0x42){return false;}

            //If the incoming length is 4 the server is asking where we are
            if(packetSize == 4 || (packetSize == 5 && _packetBuffer[4] == _id)) {
                sendMessage(remote, 0);
                return true;
            }

            //Check if the ID is correct
            if(_packetBuffer[4] != _id && _packetBuffer[4] != 255) {return false;}

            //Read the data
            if(packetSize == (9 + _packetBuffer[5 + 3])) {
                _type = _packetBuffer[5 + 0];
                _command = _packetBuffer[5 + 1];
                _value = _packetBuffer[5 + 2];
                _dataSize = _packetBuffer[5 + 3];

                for(int i = 0; i < _dataSize; i++) {
                    _data[i] = (byte)_packetBuffer[5 + 4 + i];
                }

                return true;
            }
        }
        
        return false;
    }
};

#endif