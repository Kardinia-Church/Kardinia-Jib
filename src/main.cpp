/*
Kardinia Jib Controller
By Kardinia Church 2020
main.cpp main entry point
*/

//Depending on the defined board include the relevant board
#ifdef CONTROL_PANEL
#include "controlPanel/main.h"
#endif

#ifdef LANC_CONTROLLER
#include "lancController/main.h"
#endif