// FSM.h
// Runs on LM4F120/TM4C123
// The state machine underlying the soundtrack project.
//	Based on switch input, allows a user to navigate between
//	several screens including a title screen,
//	song selection screen, and options menu.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

#include "stdint.h"

// **************Next_State*********************
// Given the current state and the current input, find the next state of the FSM.
// Input: Current state and current switch input.
// Output: Next state
uint32_t Next_State(uint32_t,uint32_t);

// **************Draw_Options*********************
// Draw options menu.
// Input: Current menu item position, a list of menu choice names, and a number of options.
// Output: None
void Draw_Options(uint8_t menupos,char* menu_Choice[],uint8_t NUMOPTIONS,int YBEGINLIST);

// **************Draw_Title*********************
// Draw title screen.
// Input: Title origin coordinates, title border size, and title string.
// Output: None
void Draw_Title(int XTITLE, int YTITLE, int TITLEBORDER,char* title);