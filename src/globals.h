#ifndef GLOBALS_H
#define GLOBALS_H

enum CommandType {
    Lanc,
    Movement,
    Control
};

enum LancCommand {
    Zoom,
    Focus,
    AutoFocus
};

enum ControlCommand {
    Reboot
};

enum MovementCommand {
    RelMove,
    AbsMove,
    MoveSpeed,
    Stop
};

#endif