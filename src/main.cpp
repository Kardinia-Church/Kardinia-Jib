/*
Kardinia Jib Controller
By Kardinia Church 2020

main.cpp main entry point
*/

//Depending on the defined board include the relevant board
#ifdef CONTROL_PANEL
#include "controlPanel/controllerHandler.h"
#endif

#ifdef HEAD
#include "head/headHandler.h"
#endif