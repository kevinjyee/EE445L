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
#define	NUM_TEMPOS								8

#define zero 0
#define three 3

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
uint32_t StartCritical (void);// previous I bit, disable interrupts
void EndCritical(uint32_t sr);// restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint8_t uninitialized = 1;
volatile uint16_t Note_Index = 0; 
volatile uint8_t I = 0;
volatile uint8_t J = 0;
volatile uint8_t current_Soprano_Beats = 0;
volatile uint8_t current_Alto_Beats = 0;
volatile uint16_t soprano_Note_Index = 0;
volatile uint16_t alto_Note_Index = 0;
volatile uint8_t letOff = 0;						// Should be high when need to leave space between notes.		
extern volatile int currentSongPos;

enum pitch{
	C1, DF1, D1, EF1, E1, F1, GF1, G1, AF1, A1, BF1, B1,
	C2, DF2, D2, EF2, E2, F2, GF2, G2, AF2, A2, BF2, B2,
	C3, DF3, D3, EF3, E3, F3, GF3, G3, AF3, A3, BF3, B3,
	C4, DF4, D4, EF4, E4, F4, GF4, G4, AF4, A4, BF4, B4,
	C5, DF5, D5, EF5, E5, F5, GF5, G5, AF5, A5, BF5, B5,
	REST, NUM_NOTES
};

const unsigned short SineUpdateDelay[NUM_NOTES] = {
	11945,11274,10641,10044,9480,8948,8446,7972,7525,7102,6704,
	6327,5972,5637,5321,5022,4740,4474,4223,3986,3762,3551,3352,
	3164,2986,2819,2660,2511,2370,2237,2112,1993,1881,1776,1676,1582,
	1493,1409,1330,1256,1185,1119,1056,997,941,888,838,791,747,705,665,
	628,593,559,528,498,470,444,419,395,0
};

enum time_value{
	ONE_HUNDRED_TWENTY_EIGHTH, SIXTY_FOURTH, THIRTY_SECOND, SIXTEENTH, EIGHTH, EIGHTH_DOT, QUARTER, QUARTER_DOT, HALF, HALF_DOT, WHOLE, NUM_DIFF_NOTE_DURATIONS
};

const uint8_t time_value[NUM_DIFF_NOTE_DURATIONS] = {
	1, 2, 4, 8, 16, 24, 32, 48, 64, 96, 128
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
  Note soprano_Notes[68];  // Soprano melody for song.
	int num_Soprano_Notes;		// Number of soprano notes.
	Note alto_Notes[103];			// Alto melody for song.
	int num_Alto_Notes;				// Number of alto notes.
} Song;  /* Song structs */

typedef struct
{
	Song songs[1];
} Song_Choices; /* Playlist for Lab 5! */

volatile Song_Choices testSongs;
static const Song New_Bark_Town;

// 1024-amplitude sine-wave.
const uint16_t Sine_Wave[WAVETABLE_LENGTH] = {
	1024, 1124, 1224, 1321, 1416, 1507, 1593, 1674, 1748, 1816, 1875, 1927, 1970,
	2004, 2028, 2043, 2047, 2043, 2028, 2004, 1970, 1927, 1875, 1816, 1748, 1674,
	1593, 1507, 1416, 1321, 1224, 1124, 1024, 924, 824, 727, 632, 541, 455, 374,
	300, 232, 173, 121, 78, 44, 20, 5, 0, 5, 20, 44, 78, 121, 173, 232, 300, 374,
	455, 541, 632, 727, 824, 924
};

void OutputSopranoWave(void){
	
  DAC_Out(Sine_Wave[I] + Sine_Wave[J]);
	uint32_t sr = StartCritical();
	I = (I + 1) % WAVETABLE_LENGTH;
	EndCritical(sr);
}

void OutputAltoWave(void){
	DAC_Out(Sine_Wave[I] + Sine_Wave[J]);
	uint32_t sr = StartCritical();
	J = (J + 1) % WAVETABLE_LENGTH;
	EndCritical(sr);
}

// ***************** Play_Song *****************
void Play_Song(void){
	Song current_Song = testSongs.songs[currentSongPos];
	
	Note current_Soprano_Note = current_Song.soprano_Notes[soprano_Note_Index]; // Get current soprano note.
	if(current_Soprano_Beats == 0){ // If at a new note, load Timer0A with new soprano frequency.
		Timer0A_Init(&OutputSopranoWave, SineUpdateDelay[current_Soprano_Note.note_pitch]);
		//Timer1_Init(&OutputSopranoWave, SineUpdateDelay[current_Soprano_Note.note_pitch]);
	} else if(current_Soprano_Beats == (time_value[current_Soprano_Note.note_time_value] - 1)){ // Update note index if moving to next note.
		soprano_Note_Index = (soprano_Note_Index + 1) % current_Song.num_Soprano_Notes;
	} else if(current_Soprano_Beats == (time_value[current_Soprano_Note.note_time_value] - 2)){
		Timer0A_Init(&OutputSopranoWave, SineUpdateDelay[REST]);
	}
	current_Soprano_Beats = (current_Soprano_Beats + 1) % time_value[current_Soprano_Note.note_time_value]; // Update beats
	
	Note current_Alto_Note = current_Song.alto_Notes[alto_Note_Index];
	if(current_Alto_Beats == 0){ // If at new note, load Timer1A with new alto frequency.
		Timer1_Init(&OutputAltoWave, SineUpdateDelay[current_Alto_Note.note_pitch]);
		//Timer0A_Init(&OutputAltoWave, SineUpdateDelay[current_Alto_Note.note_pitch]);
	} else if(current_Alto_Beats == (time_value[current_Alto_Note.note_time_value] - 1)){ // Update note index if moving to next note.
		alto_Note_Index = (alto_Note_Index + 1) % current_Song.num_Alto_Notes;
	} else if(current_Alto_Beats == (time_value[current_Alto_Note.note_time_value] - 2)){
		Timer1_Init(&OutputAltoWave, SineUpdateDelay[REST]);
	}
	current_Alto_Beats = (current_Alto_Beats + 1) % time_value[current_Alto_Note.note_time_value];
	
}

// ***************** Music_Init ****************
// Creates a dummy song.
void Music_Init(void){
	  Note  D4_8 = { D4,EIGHTH};
  Note  E4_8 = { E4,EIGHTH};
  Note  GF4_4 = { GF4,QUARTER};
  Note  A4_4 = { A4,QUARTER};
  Note  G4_8 = { G4,EIGHTH};
  Note  GF4_8 = { GF4,EIGHTH};
  Note  GF4_4dot = { GF4,QUARTER_DOT};
  Note  A3_4 = { A3,QUARTER};
  Note REST_8 = {REST,EIGHTH};
  Note  G3_16 = { G3,SIXTEENTH};
  Note  A3_16 = { A3,SIXTEENTH};
  Note  B3_4 = { B3,QUARTER};
  Note  D4_4 = { D4,QUARTER};
  Note  DF4_8 = { DF4,EIGHTH};
  Note  E4_4dot = { E4,QUARTER_DOT};
  Note  E4_4 = { E4,QUARTER};
  Note  D4_16 = { D4,SIXTEENTH};
  Note  E4_16 = { E4,SIXTEENTH};
  Note  BF4_8 = { BF4,EIGHTH};
  Note  A4_8 = { A4,EIGHTH};
  Note  A4_4dot = { A4,QUARTER_DOT};
  Note  DF5_8 = { DF5,EIGHTH};
  Note  D5_4 = { D5,QUARTER};
  Note  GF4_16 = { GF4,SIXTEENTH};
  Note  G4_4dot = { G4,QUARTER_DOT};
  Note  B4_2 = { B4,HALF};
  Note  G4_16 = { G4,SIXTEENTH};
  Note  A2_16 = { A2,SIXTEENTH};
  Note  DF3_16 = { DF3,SIXTEENTH};
  Note  E3_16 = { E3,SIXTEENTH};
  Note  DF4_16 = { DF4,SIXTEENTH};
  Note  A4_16 = { A4,SIXTEENTH};
  Note  B4_4dot = { B4,QUARTER_DOT};
  Note  G4_4 = { G4,QUARTER};
  Note  E4_1 = { E4,WHOLE};
  Note  DF5_1 = { DF5,WHOLE};
  Note  E5_2dot  = { E5,HALF_DOT};
	
	  Note REST_4 = {REST,QUARTER};
  Note  D2_4 = { D2,QUARTER};
  Note  D2_16 = { D2,SIXTEENTH};
  Note  D2_8 = { D2,EIGHTH};

  Note C3_8 = {C3,EIGHTH};
  Note  G2_4 = { G2,QUARTER};
  Note  G2_16 = { G2,SIXTEENTH};
  Note  AF2_16 = { AF2,SIXTEENTH};
  Note  AF2_4 = { AF2,QUARTER};
  Note  A2_8 = { A2,EIGHTH};
  Note  A2_4 = { A2,QUARTER};
  Note  AF1_8 = { AF1,EIGHTH};
  Note  AF1_4 = { AF1,QUARTER};
  Note  A1_8 = { A1,EIGHTH};
  Note  A1_4 = { A1,QUARTER};
  Note  DF2_8 = { DF2,EIGHTH};
  Note  G2_8 = { G2,EIGHTH};
  Note  B2_8 = { B2,EIGHTH};
  Note  D3_8 = { D3,EIGHTH};
  Note  GF3_4 = { GF3,QUARTER};
  Note  G3_4 = { G3,QUARTER};
  Note  A3_8 = { A3,EIGHTH};
  Note  E3_8 = { E3,EIGHTH};
  Note  G3_8 = { G3,EIGHTH};
	Note BF3_8 = {BF3, EIGHTH};
	Note DF3_8 = {DF3, EIGHTH};
	Note AF2_8 = {AF2, EIGHTH};
	Note A1_4dot = {A1, QUARTER_DOT};
	
	Song New_Bark_Town = {0, "New Bark Town", BPM60, {D4_8, E4_8, GF4_4, A4_4, G4_8, GF4_8, E4_8, G4_8, GF4_4dot, D4_8, A3_4, REST_8, G3_16, A3_16,
		B3_4, D4_4, E4_8, D4_8, DF4_8, D4_8, E4_4dot, GF4_8, E4_4, REST_8, D4_16, E4_16, GF4_4, A4_4, BF4_8, A4_8, G4_8, BF4_8,
		A4_4dot, DF5_8, D5_4, REST_8, E4_16, GF4_16, G4_4dot, A4_8, B4_2, A4_4dot, G4_16, GF4_16, A2_16, DF3_16, E3_16, G3_16, A3_16,
		DF4_16, E4_16, A4_16, B4_4dot, A4_4dot, G4_4, E4_1, B4_4dot, A4_4dot, D5_4, DF5_1, B4_4dot, A4_4dot, G4_4, E4_1, B4_4dot, A4_4dot, D5_4, E5_2dot},
			68, {REST_4,D2_4,REST_8,D2_16,D2_16,D2_8,D2_4,BF3_8,D2_4,REST_8,D2_16,D2_16,D2_8,D2_4,C3_8,G2_4,REST_8,G2_16,G2_16, AF2_8,AF2_4,D2_8,A3_4,
				REST_8,A3_16,A3_16,A2_8,A2_4,DF2_8,D2_4,REST_8,D2_16,D2_16,D2_8,D2_4,BF3_8,D2_4,REST_8,D2_16,D2_16,D2_8,D2_4,C3_8,G2_4, REST_8, G2_16, G2_16,
				AF1_8, AF1_4, D2_8, A2_4, REST_8, A2_16, A2_16, A1_8, A1_4dot, G2_8, B2_8, D3_8, GF3_4, G3_4, D3_8,A3_8,DF3_8,E3_8,G3_4,A3_4,E3_8,G2_8,
				B2_8, D3_8, GF3_4, G3_4, D3_8,A3_8,DF3_8,E3_8,G3_4,A3_4,E3_8,G2_8, B2_8, D3_8, GF3_4, G3_4, D3_8,A3_8,DF3_8,E3_8,G3_4,A3_4,E3_8,G2_8, B2_8,
				D3_8, GF3_4, G3_4, D3_8,A3_8,DF3_8,E3_8,G3_4,A3_8}, 103};

		
	//Song testSong1 =  {0, "TestSong1", BPM60, {REST_8, D5_4}, 2, {D5_4, D5_4}, 2};
	testSongs.songs[0] = New_Bark_Town;

}


// ***************** Play ****************
// Plays music when called.
// Inputs:  none
// Outputs: none
void Play(void){
	if(uninitialized){
		
		Music_Init();
		uninitialized = 0;
	}
	SysTick_Init(&Play_Song, tempo_Reload[testSongs.songs[currentSongPos].my_tempo]);
	
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
	SysTick_Init(&Play_Song, tempo_Reload[testSongs.songs[currentSongPos].my_tempo]);
}

// ***************** Rewind ****************
// Restart the current song.
// Inputs:  none
// Outputs: none
void Rewind(){
	Pause();
	current_Soprano_Beats = 0; current_Alto_Beats = 0;
	soprano_Note_Index = 0; alto_Note_Index = 0;
	SysTick_Init(&Play_Song, tempo_Reload[testSongs.songs[currentSongPos].my_tempo]);
}
