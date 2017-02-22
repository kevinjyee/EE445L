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
#include "SysTick.h"
#include "Music.h"

volatile uint16_t Note_Index = 0; 
volatile uint8_t I = 0;
volatile uint8_t J = 0;



typedef struct
{
	int id;								// For use in menu selection
	char name[20];					// For use in printing name to LCD.
	int tempo;						// Sets tempo of song
  int soprano_Notes[200];  // Soprano melody for song.
	int alto_Notes[200];			// Alto melody for song.
} Song;  /* Song structs */


typedef struct
{
	Song songs[1];
} Song_Choices; /* Playlist for Lab 5! */


// ***************** Play ****************
// Plays music when called.
// Inputs:  none
// Outputs: none
void Play(void){
}

// ***************** Pause ****************
// Pauses music when called.
// Inputs:  none
// Outputs: none
void Pause(void){
}

// ***************** change_Song ****************
// Changes the song that is playing or begins playing the selected song.
// Inputs:  none
// Outputs: none
void Change_Song(){
}

// ***************** Rewind ****************
// Restart the current song.
// Inputs:  none
// Outputs: none
void Rewind(){
}

// 