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

#define C3 		0
#define C3Sh 	1
#define D3		2
#define D3Sh	3
#define E3		4
#define F3		5
#define F3Sh	6
#define G3 		7
#define G3Sh	8
#define A3		9
#define A3Sh	10
#define B3		11
#define C4 		12
#define C4Sh 	13
#define D4		14
#define D4Sh	15
#define E4		16
#define F4		17
#define F4Sh	18
#define G4 		19
#define G4Sh	20
#define A4		21
#define A4Sh	22
#define B4		23
#define C5		24
#define C5Sh	25
#define D5		26
#define D5Sh  27
#define E5    28
#define F5    29
#define F5Sh  30
#define G5		31
#define C6		32
#define C7		33	
#define REST  34
#define NUM_NOTES 35

// Array of reload values to set wave frequency.
const uint16_t Notes[NUM_NOTES] = { //C3-G5
	19111, 18039, 17026, 16071, 15169, 14318, 13513, 12755, 12039, 11364,
					10726, 10124, 9555, 9019, 8513, 8035, 7584, 7159, 6757, 6378, 
					6020, 5682, 5363, 5061, 4778, 4510, 4257, 4018, 3792, 3579, 
					3378, 3189, 2389, 1194, 65535
};


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