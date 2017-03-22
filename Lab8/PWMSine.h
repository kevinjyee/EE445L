// PWMSine.h
// Runs on LM4F120/TM4C123
// Plays notes from a selected song using Pulse Width Modulation.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

#include <stdint.h>

// ***************** PWMSine_Init ****************
// Activate PWMSine_Init which calls PWM.c to set up music playing.
// Inputs:  song_Id is a number which selects a song from 5 given options.
// Outputs: none
void PMWSine_Init(uint8_t song_Id);
