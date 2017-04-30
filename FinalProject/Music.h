/* File Name:    Music.h
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      May 5, 2015 by Daniel Valvano
 * Description:  This program contains utilities for playing music.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 4/4/2017	
 */
 
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "SysTick.h"

extern volatile uint16_t Note_Index; 
extern volatile uint8_t I;
extern volatile uint8_t J;
extern volatile int currentSongPos;

void Music_Init(void);

// ***************** Play ****************
// Plays music when called.
// Inputs:  none
// Outputs: none
void Play(void);

// ***************** Pause ****************
// Pauses music when called.
// Inputs:  none
// Outputs: none
void Pause(void);

// ***************** Change_Song ****************
// Changes the song that is playing or begins playing the selected song.
// Inputs:  none
// Outputs: none
void Change_Song(void);

// ***************** Rewind ****************
// Restart the current song.
// Inputs:  none
// Outputs: none
void Rewind(void);

// ***************** Mess_With_Tempo ****************
// Changes the tempo, initializing if music hasn't been.
// Inputs:  none
// Outputs: none
void Mess_With_Tempo(uint16_t current_tempo);
