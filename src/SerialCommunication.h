/*
Kardinia Jib By Kardinia Church 2020
"LANC" is a registered trademark of SONY.
CANON calls their LANC compatible port "REMOTE".

commandStructure.h - Defines the commands sent between processors
*/

/*
Command structure is as follows:

0       1       2        3       4                                        x                 END
0x80    Type    Command  Value   Extra Data length (if required otherwise 0)    Data        0x0A (\n)
*/

#include <Arduino.h>

//The serial communication object
class SerialCommunication {
    private:
    Stream *_serial;
    int _currentType;
    int _currentCommand = -1;
    int _currentValue = -1;
    int _currentDataLength = -1;
    int _currentData[50];
    public:
    SerialCommunication(Stream *serialMonitor) {
        _serial = serialMonitor;
    }

    enum Type {
        Lanc,
        Head,
        Status
    };

    //Process the incoming serial data. Returns true when data was received
    bool processSerialCommunication() {
        int i = 0;
        bool validCommand = false;
        if(_serial->available()) {
            delay(20); //Wait for the buffer to fill
            while(_serial->available()) {
                int current = _serial->read();
                if(i == 0 && current == 0x80){validCommand = true;}
 
                if(validCommand) {
                    if(i == 1){_currentType = current;}
                    if(i == 2){_currentCommand = current;}
                    if(i == 3){_currentValue = current;}
                    if(i == 4) {
                        _currentDataLength = current;
                        for(int j = 0; j < _currentDataLength; j++){_currentData[i] = 0;}
                    }
                    if(i > 4) { 
                        _currentData[i - 5] = current;
                    }
                }

                i++;
            }
        }

        return validCommand;
    }

    //Return the currently read information
    Type getSerialType() {return (Type)_currentType;}
    int getSerialCommand() {return _currentCommand;}
    int getSerialValue() {return _currentValue;}
    int getSerialDataLength() {return _currentDataLength;}
    int *getSerialData() {return _currentData;}

    //Generate the buffer to send over the serial monitor
    void sendRawSerial(Type type, int command, int value, int dataSize = 0, int *data = nullptr) {
        _serial->write(0x80);
        
        if(data != nullptr && dataSize != 0) {
            _serial->write(type);
            _serial->write(command);
            _serial->write(value);
            _serial->write(dataSize);
            for(int i = 0; i < dataSize; i++) {_serial->write(data[i]);}
            _serial->write(0x0A);
            delete data;
        }
        else {
            _serial->write(type);
            _serial->write(command);
            _serial->write(value);
            _serial->write(dataSize);
            _serial->write(0x0A);
        }

        delay(50);
    }

    enum LancCommand {
        ZoomIn,
        ZoomOut,
        Rec,
        Focus,
        WhiteBalance,
        Iris,
        Shutter
    };

    enum LancZoomValue {
        Speed1,
        Speed2,
        Speed3,
        Speed4,
        Speed5,
        Speed6,
        Speed7,
        Speed8
    };

    enum LancRecValue {
        Start,
        Stop
    };

    enum LancFocusValue {
        Toggle,
        Far,
        Near
    };

    enum LancWhiteBalanceValue {
        Process,
        Reset
    };

    enum LancIrisValue {
        Auto,
        Open,
        Close
    };

    enum LancShutterValue {
        Next
    };

    enum StatusValue {
        Startup,
        Ready,
        NetworkDisconnected,
        NetworkConnected
    };

    //Send out a lanc command
    void sendLancCommand(LancCommand lancCommand, int value) {
        sendRawSerial(Type::Lanc, lancCommand, value);
    }

    enum HeadCommand {
        RelMove,
        AbsMove,
        MoveSpeed,
        ResetHead
    };

    //Send out a head command
    void sendHeadCommand(HeadCommand headCommand, int dataLength = 0, int *data = nullptr) {
        sendRawSerial(Type::Head, headCommand, 0, dataLength, data);
    }
    //Generate the required data to a move command
    int *generateMoveData(long x, long y, float speed, float acceleration) {
        static int ret[4 + 4 + 4 + 4];
        for(int i = 0; i < 4; i++){ret[i] = ((byte*)&x)[i];}
        for(int i = 0; i < 4; i++){ret[i + 4] = ((byte*)&y)[i];}   
        for(int i = 0; i < 4; i++){ret[i + 8] = ((byte*)&speed)[i];}
        for(int i = 0; i < 4; i++){ret[i + 12] = ((byte*)&acceleration)[i];}
        return ret;
    }

    //Generate the required data for a move speed command
    int *generateMoveSpeedData(float speedX, float speedY, float acceleration = 100.0) {
        static int ret[4 + 4 + 4];
        for(int i = 0; i < 4; i++){ret[i] = ((byte*)&speedX)[i];}
        for(int i = 0; i < 4; i++){ret[i + 4] = ((byte*)&speedY)[i];}   
        for(int i = 0; i < 4; i++){ret[i + 8] = ((byte*)&acceleration)[i];}
        return ret;
    }

    //Send a relative move to the head
    void sendRelativeMove(long x, long y, float speed, float acceleration) {
        sendHeadCommand(HeadCommand::RelMove, 16, generateMoveData(x, y, speed, acceleration));
    }

    //Send a absolute move to the head
    void sendAbsoluteMove(long x, long y, float speed, float acceleration) {
        sendHeadCommand(HeadCommand::AbsMove, 16, generateMoveData(x, y, speed, acceleration));
    }

    //Send a speed move to the head
    void sendSpeedMove(float x, float y, float accel) {
        sendHeadCommand(HeadCommand::MoveSpeed, 14, generateMoveSpeedData(x, y, accel));
    }

    //Request the head to reboot
    void sendHeadReset() {
        sendHeadCommand(HeadCommand::ResetHead);
    }

    //Send a status update
    void sendStatus(int status) {
        sendRawSerial(Type::Status, 0, status);
    }

    //Get a long value from bytes
    long getLongValue(int *buffer, int index) {
        long ret = 0;
        for(int i = index; i < index + 4; i++) {
            ret += buffer[i]<<((i - index)*8);
        }
        return ret;
    }

    //Get a flat value from bytes
    float getFloatValue(int *buffer, int index) {
        union {
            byte b[4];
            float f;
        } u;

        u.b[0] = buffer[0 + index];
        u.b[1] = buffer[1 + index];
        u.b[2] = buffer[2 + index];
        u.b[3] = buffer[3 + index];

        return  u.f;
    }
};