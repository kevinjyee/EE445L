/* File Name:    Music.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      May 5, 2015 by Daniel Valvano
 * Description:  This program contains utilities for playing music.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 2/21/2017	
 */
 
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../SysTick.h"

volatile uint8_t Note_Index = 0; 
volatile uint8_t I = 0;
volatile uint8_t J = 0;

// ***************** toggle_Play ****************
// Plays music if none playing, pauses music if already playing.
// Inputs:  none
// Outputs: none

// ***************** change_Song ****************
// Changes the song that is playing or begins playing the selected song.
// Inputs:  none
// Outputs: none

// ***************** rewind ****************
// Restart the current song.
// Inputs:  none
// Outputs: none