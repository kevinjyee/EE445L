// MainMenu.h
// Runs on LM4F120/TM4C123
// Contains all code dealing with the main menu, including
//		calls to display and management of FSM.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

#include "stdint.h"

// **************MainScreen*********************
// Calling MainScreen displays menu options and the user interface
//		with which one can navigate the Soundtrack device's options.
// Input: Most recent switch input.
// Output: Next state.
uint32_t MainScreen(uint32_t);