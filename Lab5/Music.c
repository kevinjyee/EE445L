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
#include "Timer0A.h"
#include "Timer1.h"
#include "Music.h"
#include "DAC.h"

#define WAVETABLE_LENGTH					64
#define NUM_NOTES									61
#define NUM_DIFF_NOTE_DURATIONS		9
#define	NUM_TEMPOS								8

volatile uint16_t Note_Index = 0; 
volatile uint8_t I = 0;
volatile uint8_t J = 0;
volatile uint8_t current_Soprano_Beats = 0;
volatile uint8_t current_Alto_Beats = 0;
volatile uint16_t soprano_Note_Index = 0;
volatile uint16_t alto_Note_Index = 0;
extern volatile int currentSongPos;

enum pitch{
	C0, DF0, D0, EF0, E0, F0, GF0, G0, AF0, A0, BF0, B0,
	C1, DF1, D1, EF1, E1, F1, GF1, G1, AF1, A1, BF1, B1,
	C2, DF2, D2, EF2, E2, F2, GF2, G2, AF2, A2, BF2, B2,
	C3, DF3, D3, EF3, E3, F3, GF3, G3, AF3, A3, BF3, B3,
	C4, DF4, D4, EF4, E4, F4, GF4, G4, AF4, A4, BF4, B4,
	C5, DF5, D5, EF5, E5, F5, GF5, G5, AF5, A5, BF5, B5,
	REST
};

const unsigned short SineUpdateDelay[NUM_NOTES] = {
	11945,11274,10641,10044,9480,8948,8446,7972,7525,7102,6704,
	6327,5972,5637,5321,5022,4740,4474,4223,3986,3762,3551,3352,
	3164,2986,2819,2660,2511,2370,2237,2112,1993,1881,1776,1676,1582,
	1493,1409,1330,1256,1185,1119,1056,997,941,888,838,791,747,705,665,
	628,593,559,528,498,470,444,419,395,0
};

enum time_value{
	THIRTY_SECOND, SIXTEENTH, EIGHTH, EIGHTH_DOT, QUARTER, QUARTER_DOT, HALF, HALF_DOT, WHOLE
};

const uint8_t time_value[NUM_DIFF_NOTE_DURATIONS] = {
	1, 2, 4, 6, 8, 12, 16, 24, 32
};

enum tempo{
	BPM60, BPM70, BPM80, BPM90, BPM100, BPM110, BPM120, BPM130
};

const uint32_t tempo_Reload[NUM_TEMPOS] = {
	60, 70, 80, 90, 100, 110, 120, 130
};

typedef struct
{
	enum pitch note_pitch;
	enum time_value note_time_value;
} Note;

typedef struct
{
	int id;								// For use in menu selection
	char name[20];					// For use in printing name to LCD.
	enum tempo my_tempo;						// Sets tempo of song
  Note soprano_Notes[200];  // Soprano melody for song.
	int num_Soprano_Notes;		// Number of soprano notes.
	Note alto_Notes[200];			// Alto melody for song.
	int num_Alto_Notes;				// Number of alto notes.
} Song;  /* Song structs */

typedef struct
{
	Song songs[1];
} Song_Choices; /* Playlist for Lab 5! */

Song_Choices testSongs;

// 1024-amplitude sine-wave.
const uint16_t Sine_Wave[WAVETABLE_LENGTH] = {
	1024, 1124, 1224, 1321, 1416, 1507, 1593, 1674, 1748, 1816, 1875, 1927, 1970,
	2004, 2028, 2043, 2048, 2043, 2028, 2004, 1970, 1927, 1875, 1816, 1748, 1674,
	1593, 1507, 1416, 1321, 1224, 1124, 1024, 924, 824, 727, 632, 541, 455, 374,
	300, 232, 173, 121, 78, 44, 20, 5, 0, 5, 20, 44, 78, 121, 173, 232, 300, 374,
	455, 541, 632, 727, 824, 924
};

void OutputSopranoWave(void){
  DAC_Out(Sine_Wave[I] + Sine_Wave[J]);
	I = (I + 1) % WAVETABLE_LENGTH;
}

void OutputAltoWave(void){
	DAC_Out(Sine_Wave[I] + Sine_Wave[J]);
	J = (J + 1) % WAVETABLE_LENGTH;
}

// ***************** Play_Song *****************
void Play_Song(void){
	Song current_Song = testSongs.songs[currentSongPos];
	
	Note current_Soprano_Note = current_Song.soprano_Notes[soprano_Note_Index]; // Get current soprano note.
	if(current_Soprano_Beats == 0){ // If at a new note, load Timer0A with new soprano frequency.
		Timer0A_Init(&OutputSopranoWave, SineUpdateDelay[current_Soprano_Note.note_pitch]);
	} else if(current_Soprano_Beats == time_value[current_Soprano_Note.note_time_value]){ // Update note index if moving to next note.
		soprano_Note_Index = (soprano_Note_Index + 1) % current_Song.num_Soprano_Notes;
	}
	current_Soprano_Beats = (current_Soprano_Beats + 1) % time_value[current_Soprano_Note.note_time_value]; // Update beats
	
	Note current_Alto_Note = current_Song.alto_Notes[alto_Note_Index];
	if(current_Alto_Beats == 0){ // If at new note, load Timer1A with new alto frequency.
		Timer1_Init(&OutputAltoWave, SineUpdateDelay[current_Alto_Note.note_pitch]);
	} else if(current_Alto_Beats == time_value[current_Alto_Note.note_time_value]){ // Update note index if moving to next note.
		alto_Note_Index = (alto_Note_Index + 1) % current_Song.num_Alto_Notes;
	}
	current_Alto_Beats = (current_Alto_Beats + 1) % time_value[current_Alto_Note.note_time_value];
	
}

// ***************** Music_Init ****************
// Creates a dummy song.
void Music_Init(void){
	
	Note testNote1 = {C4, WHOLE};
	Note testNote2 = {D4, WHOLE};

	Note testNote3 = {C2, HALF};
	Note testNote4 = {B1, WHOLE};
	
	Song testSong1 =  {0, "TestSong1", BPM100, {testNote1, testNote2}, 2, {testNote3, testNote4}, 2};
	
	uint8_t NBT_ID = 1;
	uint16_t NUM_NBT_SOPRANO_NOTES = 50;
	uint16_t NUM_NBT_ALTO_NOTES = 50;
	
	Note REST_8 = {REST, EIGHTH};
	Note REST_4 = {REST, QUARTER};
	Note REST_1 = {REST, WHOLE};
	
	Note G3_16 = {G3, SIXTEENTH};
	Note A3_16 = {A3, SIXTEENTH};
	Note A3_4 = {A3, QUARTER};
	Note B3_4 = {B3, QUARTER};
	Note DF4_8 = {DF4, EIGHTH};
	Note D4_16 = {D4, SIXTEENTH};
	Note D4_8 = {D4, EIGHTH};
	Note D4_4 = {D4, QUARTER};
	Note E4_16 = {E4, SIXTEENTH};
	Note E4_8 = {E4, EIGHTH};
	Note E4_4 = {E4, QUARTER};
	Note E4_4dot = {E4, QUARTER_DOT};
	Note GF4_16 = {GF4, SIXTEENTH};
	Note GF4_8 = {GF4, EIGHTH};
	Note GF4_4 = {GF4, QUARTER};
	Note GF4_4dot = {GF4, QUARTER_DOT};
	Note G4_16 = {GF4, SIXTEENTH};
	Note G4_8 = {G4, EIGHTH};
	Note G4_4dot = {G4, QUARTER_DOT};
	Note A4_8 = {A4, EIGHTH};
	Note A4_4 = {A4, QUARTER};
	Note A4_4dot = {A4, QUARTER_DOT};
	Note BF4_8 = {BF4, EIGHTH};
	Note B4_2 = {B4, HALF};
	Note DF5_8 = {DF5, EIGHTH};
	Note D5_4 = {D5, QUARTER};
	
	
	Song New_Bark_Town = {NBT_ID, "New Bark Town", BPM100, {E4_4, D4_4, A3_4}, 3, {E4_4, D4_4, A3_4}, 3};
	testSongs.songs[0] = New_Bark_Town;

}


// ***************** Play ****************
// Plays music when called.
// Inputs:  none
// Outputs: none
void Play(void){
	SysTick_Init(&Play_Song, tempo_Reload[BPM100]);
	
}

// ***************** Pause ****************
// Pauses music when called.
// Inputs:  none
// Outputs: none
void Pause(void){
	SysTick_Halt();
	Timer0A_Halt();
	Timer1A_Halt();
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
	Pause();
	current_Soprano_Beats = 0; current_Alto_Beats = 0;
	soprano_Note_Index = 0; alto_Note_Index = 0;
	SysTick_Init(&Play_Song, tempo_Reload[BPM100]);
}
