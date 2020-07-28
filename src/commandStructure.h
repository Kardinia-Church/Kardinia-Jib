/*
Kardinia Jib By Kardinia Church 2020
"LANC" is a registered trademark of SONY.
CANON calls their LANC compatible port "REMOTE".

commandStructure.h - Defines the commands sent between processors
*/

/*
Control <> Lanc
COMMAND::VALUE

Eg for zoom in at full speed
ZIN::8
*/

#include <Arduino.h>

enum LancCommand {
    ZoomIn,
    ZoomOut,
    Rec,
    Focus,
    WhiteBalance,
    Iris,
    Shutter
};
#define LANC_COMMAND {"ZIN", "ZOUT","RECO", "FOCU", "WBAL", "IRIS", "SHUT"}

enum RecValue {
    Start,
    Stop
};
#define REC_VALUE {"B", "E"}

enum FocusValue {
    Toggle,
    Far,
    Near
};
#define FOCUS_VALUE = {"A", "F", "N"};

enum WhiteBalanceValue {
    Process,
    Reset
};
#define WHITEBAL_VALUE = {"T", "R"};

enum IrisValue {
    Auto,
    Open,
    Close
};
#define IRIS_VALUE = {"A", "O", "C"};

enum ShutterValue {
    Next
};
#define SHUTTER_VALUE = {"N"};

//Generate the lanc command to send out
String generateLancCommand(String command, String value) {
    return command + "::" + value;
};

//Read the lanc command from the incoming buffer
LancCommand findLancCommand(String incoming) {
    if(incoming.length() == 6) {
        for(int i = 0; i < (sizeof(LANC_COMMAND) / sizeof(LANC_COMMAND[0])); i++) {
            if(incoming.substring(0, 3) == LANC_COMMAND[i]) {
                return LancCommand[i];
            }
        }
    }
}