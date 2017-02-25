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
  Note soprano_Notes[10];  // Soprano melody for song.
	int num_Soprano_Notes;		// Number of soprano notes.
	Note alto_Notes[10];			// Alto melody for song.
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
	2004, 2028, 2043, 2045, 2043, 2028, 2004, 1970, 1927, 1875, 1816, 1748, 1674,
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
	
	  Note C1_1 = {C1,WHOLE};
  Note  DF1_1 = { DF1,WHOLE};
  Note  D1_1 = { D1,WHOLE};
  Note  EF1_1 = { EF1,WHOLE};
  Note  E1_1 = { E1,WHOLE};
  Note  F1_1 = { F1,WHOLE};
  Note  GF1_1 = { GF1,WHOLE};
  Note  G1_1 = { G1,WHOLE};
  Note  AF1_1 = { AF1,WHOLE};
  Note  A1_1 = { A1,WHOLE};
  Note  BF1_1 = { BF1,WHOLE};
  Note  B1_1 = { B1,WHOLE};
  Note C2_1 = {C2,WHOLE};
  Note  DF2_1 = { DF2,WHOLE};
  Note  D2_1 = { D2,WHOLE};
  Note  EF2_1 = { EF2,WHOLE};
  Note  E2_1 = { E2,WHOLE};
  Note  F2_1 = { F2,WHOLE};
  Note  GF2_1 = { GF2,WHOLE};
  Note  G2_1 = { G2,WHOLE};
  Note  AF2_1 = { AF2,WHOLE};
  Note  A2_1 = { A2,WHOLE};
  Note  BF2_1 = { BF2,WHOLE};
  Note  B2_1 = { B2,WHOLE};
  Note C3_1 = {C3,WHOLE};
  Note  DF3_1 = { DF3,WHOLE};
  Note  D3_1 = { D3,WHOLE};
  Note  EF3_1 = { EF3,WHOLE};
  Note  E3_1 = { E3,WHOLE};
  Note  F3_1 = { F3,WHOLE};
  Note  GF3_1 = { GF3,WHOLE};
  Note  G3_1 = { G3,WHOLE};
  Note  AF3_1 = { AF3,WHOLE};
  Note  A3_1 = { A3,WHOLE};
  Note  BF3_1 = { BF3,WHOLE};
  Note  B3_1 = { B3,WHOLE};
  Note C4_1 = {C4,WHOLE};
  Note  DF4_1 = { DF4,WHOLE};
  Note  D4_1 = { D4,WHOLE};
  Note  EF4_1 = { EF4,WHOLE};
  Note  E4_1 = { E4,WHOLE};
  Note  F4_1 = { F4,WHOLE};
  Note  GF4_1 = { GF4,WHOLE};
  Note  G4_1 = { G4,WHOLE};
  Note  AF4_1 = { AF4,WHOLE};
  Note  A4_1 = { A4,WHOLE};
  Note  BF4_1 = { BF4,WHOLE};
  Note  B4_1 = { B4,WHOLE};
  Note C5_1 = {C5,WHOLE};
  Note  DF5_1 = { DF5,WHOLE};
  Note  D5_1 = { D5,WHOLE};
  Note  EF5_1 = { EF5,WHOLE};
  Note  E5_1 = { E5,WHOLE};
  Note  F5_1 = { F5,WHOLE};
  Note  GF5_1 = { GF5,WHOLE};
  Note  G5_1 = { G5,WHOLE};
  Note  AF5_1 = { AF5,WHOLE};
  Note  A5_1 = { A5,WHOLE};
  Note  BF5_1 = { BF5,WHOLE};
  Note  B5_1 = { B5,WHOLE};
  Note REST_1 = {REST,WHOLE};
  Note C1_2 = {C1,HALF};
  Note  DF1_2 = { DF1,HALF};
  Note  D1_2 = { D1,HALF};
  Note  EF1_2 = { EF1,HALF};
  Note  E1_2 = { E1,HALF};
  Note  F1_2 = { F1,HALF};
  Note  GF1_2 = { GF1,HALF};
  Note  G1_2 = { G1,HALF};
  Note  AF1_2 = { AF1,HALF};
  Note  A1_2 = { A1,HALF};
  Note  BF1_2 = { BF1,HALF};
  Note  B1_2 = { B1,HALF};
  Note C2_2 = {C2,HALF};
  Note  DF2_2 = { DF2,HALF};
  Note  D2_2 = { D2,HALF};
  Note  EF2_2 = { EF2,HALF};
  Note  E2_2 = { E2,HALF};
  Note  F2_2 = { F2,HALF};
  Note  GF2_2 = { GF2,HALF};
  Note  G2_2 = { G2,HALF};
  Note  AF2_2 = { AF2,HALF};
  Note  A2_2 = { A2,HALF};
  Note  BF2_2 = { BF2,HALF};
  Note  B2_2 = { B2,HALF};
  Note C3_2 = {C3,HALF};
  Note  DF3_2 = { DF3,HALF};
  Note  D3_2 = { D3,HALF};
  Note  EF3_2 = { EF3,HALF};
  Note  E3_2 = { E3,HALF};
  Note  F3_2 = { F3,HALF};
  Note  GF3_2 = { GF3,HALF};
  Note  G3_2 = { G3,HALF};
  Note  AF3_2 = { AF3,HALF};
  Note  A3_2 = { A3,HALF};
  Note  BF3_2 = { BF3,HALF};
  Note  B3_2 = { B3,HALF};
  Note C4_2 = {C4,HALF};
  Note  DF4_2 = { DF4,HALF};
  Note  D4_2 = { D4,HALF};
  Note  EF4_2 = { EF4,HALF};
  Note  E4_2 = { E4,HALF};
  Note  F4_2 = { F4,HALF};
  Note  GF4_2 = { GF4,HALF};
  Note  G4_2 = { G4,HALF};
  Note  AF4_2 = { AF4,HALF};
  Note  A4_2 = { A4,HALF};
  Note  BF4_2 = { BF4,HALF};
  Note  B4_2 = { B4,HALF};
  Note C5_2 = {C5,HALF};
  Note  DF5_2 = { DF5,HALF};
  Note  D5_2 = { D5,HALF};
  Note  EF5_2 = { EF5,HALF};
  Note  E5_2 = { E5,HALF};
  Note  F5_2 = { F5,HALF};
  Note  GF5_2 = { GF5,HALF};
  Note  G5_2 = { G5,HALF};
  Note  AF5_2 = { AF5,HALF};
  Note  A5_2 = { A5,HALF};
  Note  BF5_2 = { BF5,HALF};
  Note  B5_2 = { B5,HALF};
  Note REST_2 = {REST,HALF};
  Note C1_2dot = {C1,HALF_DOT};
  Note  DF1_2dot  = { DF1,HALF_DOT};
  Note  D1_2dot  = { D1,HALF_DOT};
  Note  EF1_2dot  = { EF1,HALF_DOT};
  Note  E1_2dot  = { E1,HALF_DOT};
  Note  F1_2dot  = { F1,HALF_DOT};
  Note  GF1_2dot  = { GF1,HALF_DOT};
  Note  G1_2dot  = { G1,HALF_DOT};
  Note  AF1_2dot  = { AF1,HALF_DOT};
  Note  A1_2dot  = { A1,HALF_DOT};
  Note  BF1_2dot  = { BF1,HALF_DOT};
  Note  B1_2dot  = { B1,HALF_DOT};
  Note C2_2dot  = {C2,HALF_DOT};
  Note  DF2_2dot  = { DF2,HALF_DOT};
  Note  D2_2dot  = { D2,HALF_DOT};
  Note  EF2_2dot  = { EF2,HALF_DOT};
  Note  E2_2dot  = { E2,HALF_DOT};
  Note  F2_2dot  = { F2,HALF_DOT};
  Note  GF2_2dot  = { GF2,HALF_DOT};
  Note  G2_2dot  = { G2,HALF_DOT};
  Note  AF2_2dot  = { AF2,HALF_DOT};
  Note  A2_2dot  = { A2,HALF_DOT};
  Note  BF2_2dot  = { BF2,HALF_DOT};
  Note  B2_2dot  = { B2,HALF_DOT};
  Note C3_2dot  = {C3,HALF_DOT};
  Note  DF3_2dot  = { DF3,HALF_DOT};
  Note  D3_2dot  = { D3,HALF_DOT};
  Note  EF3_2dot  = { EF3,HALF_DOT};
  Note  E3_2dot  = { E3,HALF_DOT};
  Note  F3_2dot  = { F3,HALF_DOT};
  Note  GF3_2dot  = { GF3,HALF_DOT};
  Note  G3_2dot  = { G3,HALF_DOT};
  Note  AF3_2dot  = { AF3,HALF_DOT};
  Note  A3_2dot  = { A3,HALF_DOT};
  Note  BF3_2dot  = { BF3,HALF_DOT};
  Note  B3_2dot  = { B3,HALF_DOT};
  Note C4_2dot  = {C4,HALF_DOT};
  Note  DF4_2dot  = { DF4,HALF_DOT};
  Note  D4_2dot  = { D4,HALF_DOT};
  Note  EF4_2dot  = { EF4,HALF_DOT};
  Note  E4_2dot  = { E4,HALF_DOT};
  Note  F4_2dot  = { F4,HALF_DOT};
  Note  GF4_2dot  = { GF4,HALF_DOT};
  Note  G4_2dot  = { G4,HALF_DOT};
  Note  AF4_2dot  = { AF4,HALF_DOT};
  Note  A4_2dot  = { A4,HALF_DOT};
  Note  BF4_2dot  = { BF4,HALF_DOT};
  Note  B4_2dot  = { B4,HALF_DOT};
  Note C5_2dot  = {C5,HALF_DOT};
  Note  DF5_2dot  = { DF5,HALF_DOT};
  Note  D5_2dot  = { D5,HALF_DOT};
  Note  EF5_2dot  = { EF5,HALF_DOT};
  Note  E5_2dot  = { E5,HALF_DOT};
  Note  F5_2dot  = { F5,HALF_DOT};
  Note  GF5_2dot  = { GF5,HALF_DOT};
  Note  G5_2dot  = { G5,HALF_DOT};
  Note  AF5_2dot  = { AF5,HALF_DOT};
  Note  A5_2dot  = { A5,HALF_DOT};
  Note  BF5_2dot  = { BF5,HALF_DOT};
  Note  B5_2dot  = { B5,HALF_DOT};
  Note REST_2dot  = {REST,HALF_DOT};
  Note C1_4 = {C1,QUARTER};
  Note  DF1_4 = { DF1,QUARTER};
  Note  D1_4 = { D1,QUARTER};
  Note  EF1_4 = { EF1,QUARTER};
  Note  E1_4 = { E1,QUARTER};
  Note  F1_4 = { F1,QUARTER};
  Note  GF1_4 = { GF1,QUARTER};
  Note  G1_4 = { G1,QUARTER};
  Note  AF1_4 = { AF1,QUARTER};
  Note  A1_4 = { A1,QUARTER};
  Note  BF1_4 = { BF1,QUARTER};
  Note  B1_4 = { B1,QUARTER};
  Note C2_4 = {C2,QUARTER};
  Note  DF2_4 = { DF2,QUARTER};
  Note  D2_4 = { D2,QUARTER};
  Note  EF2_4 = { EF2,QUARTER};
  Note  E2_4 = { E2,QUARTER};
  Note  F2_4 = { F2,QUARTER};
  Note  GF2_4 = { GF2,QUARTER};
  Note  G2_4 = { G2,QUARTER};
  Note  AF2_4 = { AF2,QUARTER};
  Note  A2_4 = { A2,QUARTER};
  Note  BF2_4 = { BF2,QUARTER};
  Note  B2_4 = { B2,QUARTER};
  Note C3_4 = {C3,QUARTER};
  Note  DF3_4 = { DF3,QUARTER};
  Note  D3_4 = { D3,QUARTER};
  Note  EF3_4 = { EF3,QUARTER};
  Note  E3_4 = { E3,QUARTER};
  Note  F3_4 = { F3,QUARTER};
  Note  GF3_4 = { GF3,QUARTER};
  Note  G3_4 = { G3,QUARTER};
  Note  AF3_4 = { AF3,QUARTER};
  Note  A3_4 = { A3,QUARTER};
  Note  BF3_4 = { BF3,QUARTER};
  Note  B3_4 = { B3,QUARTER};
  Note C4_4 = {C4,QUARTER};
  Note  DF4_4 = { DF4,QUARTER};
  Note  D4_4 = { D4,QUARTER};
  Note  EF4_4 = { EF4,QUARTER};
  Note  E4_4 = { E4,QUARTER};
  Note  F4_4 = { F4,QUARTER};
  Note  GF4_4 = { GF4,QUARTER};
  Note  G4_4 = { G4,QUARTER};
  Note  AF4_4 = { AF4,QUARTER};
  Note  A4_4 = { A4,QUARTER};
  Note  BF4_4 = { BF4,QUARTER};
  Note  B4_4 = { B4,QUARTER};
  Note C5_4 = {C5,QUARTER};
  Note  DF5_4 = { DF5,QUARTER};
  Note  D5_4 = { D5,QUARTER};
  Note  EF5_4 = { EF5,QUARTER};
  Note  E5_4 = { E5,QUARTER};
  Note  F5_4 = { F5,QUARTER};
  Note  GF5_4 = { GF5,QUARTER};
  Note  G5_4 = { G5,QUARTER};
  Note  AF5_4 = { AF5,QUARTER};
  Note  A5_4 = { A5,QUARTER};
  Note  BF5_4 = { BF5,QUARTER};
  Note  B5_4 = { B5,QUARTER};
  Note REST_4 = {REST,QUARTER};
  Note C1_4dot = {C1,QUARTER_DOT};
  Note  DF1_4dot = { DF1,QUARTER_DOT};
  Note  D1_4dot = { D1,QUARTER_DOT};
  Note  EF1_4dot = { EF1,QUARTER_DOT};
  Note  E1_4dot = { E1,QUARTER_DOT};
  Note  F1_4dot = { F1,QUARTER_DOT};
  Note  GF1_4dot = { GF1,QUARTER_DOT};
  Note  G1_4dot = { G1,QUARTER_DOT};
  Note  AF1_4dot = { AF1,QUARTER_DOT};
  Note  A1_4dot = { A1,QUARTER_DOT};
  Note  BF1_4dot = { BF1,QUARTER_DOT};
  Note  B1_4dot = { B1,QUARTER_DOT};
  Note C2_4dot = {C2,QUARTER_DOT};
  Note  DF2_4dot = { DF2,QUARTER_DOT};
  Note  D2_4dot = { D2,QUARTER_DOT};
  Note  EF2_4dot = { EF2,QUARTER_DOT};
  Note  E2_4dot = { E2,QUARTER_DOT};
  Note  F2_4dot = { F2,QUARTER_DOT};
  Note  GF2_4dot = { GF2,QUARTER_DOT};
  Note  G2_4dot = { G2,QUARTER_DOT};
  Note  AF2_4dot = { AF2,QUARTER_DOT};
  Note  A2_4dot = { A2,QUARTER_DOT};
  Note  BF2_4dot = { BF2,QUARTER_DOT};
  Note  B2_4dot = { B2,QUARTER_DOT};
  Note C3_4dot = {C3,QUARTER_DOT};
  Note  DF3_4dot = { DF3,QUARTER_DOT};
  Note  D3_4dot = { D3,QUARTER_DOT};
  Note  EF3_4dot = { EF3,QUARTER_DOT};
  Note  E3_4dot = { E3,QUARTER_DOT};
  Note  F3_4dot = { F3,QUARTER_DOT};
  Note  GF3_4dot = { GF3,QUARTER_DOT};
  Note  G3_4dot = { G3,QUARTER_DOT};
  Note  AF3_4dot = { AF3,QUARTER_DOT};
  Note  A3_4dot = { A3,QUARTER_DOT};
  Note  BF3_4dot = { BF3,QUARTER_DOT};
  Note  B3_4dot = { B3,QUARTER_DOT};
  Note C4_4dot = {C4,QUARTER_DOT};
  Note  DF4_4dot = { DF4,QUARTER_DOT};
  Note  D4_4dot = { D4,QUARTER_DOT};
  Note  EF4_4dot = { EF4,QUARTER_DOT};
  Note  E4_4dot = { E4,QUARTER_DOT};
  Note  F4_4dot = { F4,QUARTER_DOT};
  Note  GF4_4dot = { GF4,QUARTER_DOT};
  Note  G4_4dot = { G4,QUARTER_DOT};
  Note  AF4_4dot = { AF4,QUARTER_DOT};
  Note  A4_4dot = { A4,QUARTER_DOT};
  Note  BF4_4dot = { BF4,QUARTER_DOT};
  Note  B4_4dot = { B4,QUARTER_DOT};
  Note C5_4dot = {C5,QUARTER_DOT};
  Note  DF5_4dot = { DF5,QUARTER_DOT};
  Note  D5_4dot = { D5,QUARTER_DOT};
  Note  EF5_4dot = { EF5,QUARTER_DOT};
  Note  E5_4dot = { E5,QUARTER_DOT};
  Note  F5_4dot = { F5,QUARTER_DOT};
  Note  GF5_4dot = { GF5,QUARTER_DOT};
  Note  G5_4dot = { G5,QUARTER_DOT};
  Note  AF5_4dot = { AF5,QUARTER_DOT};
  Note  A5_4dot = { A5,QUARTER_DOT};
  Note  BF5_4dot = { BF5,QUARTER_DOT};
  Note  B5_4dot = { B5,QUARTER_DOT};
  Note REST_4dot = {REST,QUARTER_DOT};
  Note C1_8 = {C1,EIGHTH};
  Note  DF1_8 = { DF1,EIGHTH};
  Note  D1_8 = { D1,EIGHTH};
  Note  EF1_8 = { EF1,EIGHTH};
  Note  E1_8 = { E1,EIGHTH};
  Note  F1_8 = { F1,EIGHTH};
  Note  GF1_8 = { GF1,EIGHTH};
  Note  G1_8 = { G1,EIGHTH};
  Note  AF1_8 = { AF1,EIGHTH};
  Note  A1_8 = { A1,EIGHTH};
  Note  BF1_8 = { BF1,EIGHTH};
  Note  B1_8 = { B1,EIGHTH};
  Note C2_8 = {C2,EIGHTH};
  Note  DF2_8 = { DF2,EIGHTH};
  Note  D2_8 = { D2,EIGHTH};
  Note  EF2_8 = { EF2,EIGHTH};
  Note  E2_8 = { E2,EIGHTH};
  Note  F2_8 = { F2,EIGHTH};
  Note  GF2_8 = { GF2,EIGHTH};
  Note  G2_8 = { G2,EIGHTH};
  Note  AF2_8 = { AF2,EIGHTH};
  Note  A2_8 = { A2,EIGHTH};
  Note  BF2_8 = { BF2,EIGHTH};
  Note  B2_8 = { B2,EIGHTH};
  Note C3_8 = {C3,EIGHTH};
  Note  DF3_8 = { DF3,EIGHTH};
  Note  D3_8 = { D3,EIGHTH};
  Note  EF3_8 = { EF3,EIGHTH};
  Note  E3_8 = { E3,EIGHTH};
  Note  F3_8 = { F3,EIGHTH};
  Note  GF3_8 = { GF3,EIGHTH};
  Note  G3_8 = { G3,EIGHTH};
  Note  AF3_8 = { AF3,EIGHTH};
  Note  A3_8 = { A3,EIGHTH};
  Note  BF3_8 = { BF3,EIGHTH};
  Note  B3_8 = { B3,EIGHTH};
  Note C4_8 = {C4,EIGHTH};
  Note  DF4_8 = { DF4,EIGHTH};
  Note  D4_8 = { D4,EIGHTH};
  Note  EF4_8 = { EF4,EIGHTH};
  Note  E4_8 = { E4,EIGHTH};
  Note  F4_8 = { F4,EIGHTH};
  Note  GF4_8 = { GF4,EIGHTH};
  Note  G4_8 = { G4,EIGHTH};
  Note  AF4_8 = { AF4,EIGHTH};
  Note  A4_8 = { A4,EIGHTH};
  Note  BF4_8 = { BF4,EIGHTH};
  Note  B4_8 = { B4,EIGHTH};
  Note C5_8 = {C5,EIGHTH};
  Note  DF5_8 = { DF5,EIGHTH};
  Note  D5_8 = { D5,EIGHTH};
  Note  EF5_8 = { EF5,EIGHTH};
  Note  E5_8 = { E5,EIGHTH};
  Note  F5_8 = { F5,EIGHTH};
  Note  GF5_8 = { GF5,EIGHTH};
  Note  G5_8 = { G5,EIGHTH};
  Note  AF5_8 = { AF5,EIGHTH};
  Note  A5_8 = { A5,EIGHTH};
  Note  BF5_8 = { BF5,EIGHTH};
  Note  B5_8 = { B5,EIGHTH};
  Note REST_8 = {REST,EIGHTH};
  Note C1_8dot = {C1,EIGHTH_DOT};
  Note  DF1_8dot = { DF1,EIGHTH_DOT};
  Note  D1_8dot = { D1,EIGHTH_DOT};
  Note  EF1_8dot = { EF1,EIGHTH_DOT};
  Note  E1_8dot = { E1,EIGHTH_DOT};
  Note  F1_8dot = { F1,EIGHTH_DOT};
  Note  GF1_8dot = { GF1,EIGHTH_DOT};
  Note  G1_8dot = { G1,EIGHTH_DOT};
  Note  AF1_8dot = { AF1,EIGHTH_DOT};
  Note  A1_8dot = { A1,EIGHTH_DOT};
  Note  BF1_8dot = { BF1,EIGHTH_DOT};
  Note  B1_8dot = { B1,EIGHTH_DOT};
  Note C2_8dot = {C2,EIGHTH_DOT};
  Note  DF2_8dot = { DF2,EIGHTH_DOT};
  Note  D2_8dot = { D2,EIGHTH_DOT};
  Note  EF2_8dot = { EF2,EIGHTH_DOT};
  Note  E2_8dot = { E2,EIGHTH_DOT};
  Note  F2_8dot = { F2,EIGHTH_DOT};
  Note  GF2_8dot = { GF2,EIGHTH_DOT};
  Note  G2_8dot = { G2,EIGHTH_DOT};
  Note  AF2_8dot = { AF2,EIGHTH_DOT};
  Note  A2_8dot = { A2,EIGHTH_DOT};
  Note  BF2_8dot = { BF2,EIGHTH_DOT};
  Note  B2_8dot = { B2,EIGHTH_DOT};
  Note C3_8dot = {C3,EIGHTH_DOT};
  Note  DF3_8dot = { DF3,EIGHTH_DOT};
  Note  D3_8dot = { D3,EIGHTH_DOT};
  Note  EF3_8dot = { EF3,EIGHTH_DOT};
  Note  E3_8dot = { E3,EIGHTH_DOT};
  Note  F3_8dot = { F3,EIGHTH_DOT};
  Note  GF3_8dot = { GF3,EIGHTH_DOT};
  Note  G3_8dot = { G3,EIGHTH_DOT};
  Note  AF3_8dot = { AF3,EIGHTH_DOT};
  Note  A3_8dot = { A3,EIGHTH_DOT};
  Note  BF3_8dot = { BF3,EIGHTH_DOT};
  Note  B3_8dot = { B3,EIGHTH_DOT};
  Note C4_8dot = {C4,EIGHTH_DOT};
  Note  DF4_8dot = { DF4,EIGHTH_DOT};
  Note  D4_8dot = { D4,EIGHTH_DOT};
  Note  EF4_8dot = { EF4,EIGHTH_DOT};
  Note  E4_8dot = { E4,EIGHTH_DOT};
  Note  F4_8dot = { F4,EIGHTH_DOT};
  Note  GF4_8dot = { GF4,EIGHTH_DOT};
  Note  G4_8dot = { G4,EIGHTH_DOT};
  Note  AF4_8dot = { AF4,EIGHTH_DOT};
  Note  A4_8dot = { A4,EIGHTH_DOT};
  Note  BF4_8dot = { BF4,EIGHTH_DOT};
  Note  B4_8dot = { B4,EIGHTH_DOT};
  Note C5_8dot = {C5,EIGHTH_DOT};
  Note  DF5_8dot = { DF5,EIGHTH_DOT};
  Note  D5_8dot = { D5,EIGHTH_DOT};
  Note  EF5_8dot = { EF5,EIGHTH_DOT};
  Note  E5_8dot = { E5,EIGHTH_DOT};
  Note  F5_8dot = { F5,EIGHTH_DOT};
  Note  GF5_8dot = { GF5,EIGHTH_DOT};
  Note  G5_8dot = { G5,EIGHTH_DOT};
  Note  AF5_8dot = { AF5,EIGHTH_DOT};
  Note  A5_8dot = { A5,EIGHTH_DOT};
  Note  BF5_8dot = { BF5,EIGHTH_DOT};
  Note  B5_8dot = { B5,EIGHTH_DOT};
  Note REST_8dot = {REST,EIGHTH_DOT};
  Note C1_16 = {C1,SIXTEENTH};
  Note  DF1_16 = { DF1,SIXTEENTH};
  Note  D1_16 = { D1,SIXTEENTH};
  Note  EF1_16 = { EF1,SIXTEENTH};
  Note  E1_16 = { E1,SIXTEENTH};
  Note  F1_16 = { F1,SIXTEENTH};
  Note  GF1_16 = { GF1,SIXTEENTH};
  Note  G1_16 = { G1,SIXTEENTH};
  Note  AF1_16 = { AF1,SIXTEENTH};
  Note  A1_16 = { A1,SIXTEENTH};
  Note  BF1_16 = { BF1,SIXTEENTH};
  Note  B1_16 = { B1,SIXTEENTH};
  Note C2_16 = {C2,SIXTEENTH};
  Note  DF2_16 = { DF2,SIXTEENTH};
  Note  D2_16 = { D2,SIXTEENTH};
  Note  EF2_16 = { EF2,SIXTEENTH};
  Note  E2_16 = { E2,SIXTEENTH};
  Note  F2_16 = { F2,SIXTEENTH};
  Note  GF2_16 = { GF2,SIXTEENTH};
  Note  G2_16 = { G2,SIXTEENTH};
  Note  AF2_16 = { AF2,SIXTEENTH};
  Note  A2_16 = { A2,SIXTEENTH};
  Note  BF2_16 = { BF2,SIXTEENTH};
  Note  B2_16 = { B2,SIXTEENTH};
  Note C3_16 = {C3,SIXTEENTH};
  Note  DF3_16 = { DF3,SIXTEENTH};
  Note  D3_16 = { D3,SIXTEENTH};
  Note  EF3_16 = { EF3,SIXTEENTH};
  Note  E3_16 = { E3,SIXTEENTH};
  Note  F3_16 = { F3,SIXTEENTH};
  Note  GF3_16 = { GF3,SIXTEENTH};
  Note  G3_16 = { G3,SIXTEENTH};
  Note  AF3_16 = { AF3,SIXTEENTH};
  Note  A3_16 = { A3,SIXTEENTH};
  Note  BF3_16 = { BF3,SIXTEENTH};
  Note  B3_16 = { B3,SIXTEENTH};
  Note C4_16 = {C4,SIXTEENTH};
  Note  DF4_16 = { DF4,SIXTEENTH};
  Note  D4_16 = { D4,SIXTEENTH};
  Note  EF4_16 = { EF4,SIXTEENTH};
  Note  E4_16 = { E4,SIXTEENTH};
  Note  F4_16 = { F4,SIXTEENTH};
  Note  GF4_16 = { GF4,SIXTEENTH};
  Note  G4_16 = { G4,SIXTEENTH};
  Note  AF4_16 = { AF4,SIXTEENTH};
  Note  A4_16 = { A4,SIXTEENTH};
  Note  BF4_16 = { BF4,SIXTEENTH};
  Note  B4_16 = { B4,SIXTEENTH};
  Note C5_16 = {C5,SIXTEENTH};
  Note  DF5_16 = { DF5,SIXTEENTH};
  Note  D5_16 = { D5,SIXTEENTH};
  Note  EF5_16 = { EF5,SIXTEENTH};
  Note  E5_16 = { E5,SIXTEENTH};
  Note  F5_16 = { F5,SIXTEENTH};
  Note  GF5_16 = { GF5,SIXTEENTH};
  Note  G5_16 = { G5,SIXTEENTH};
  Note  AF5_16 = { AF5,SIXTEENTH};
  Note  A5_16 = { A5,SIXTEENTH};
  Note  BF5_16 = { BF5,SIXTEENTH};
  Note  B5_16 = { B5,SIXTEENTH};
  Note REST_16 = {REST,SIXTEENTH};
  Note C1_32 = {C1,THIRTY_SECOND};
  Note  DF1_32 = { DF1,THIRTY_SECOND};
  Note  D1_32 = { D1,THIRTY_SECOND};
  Note  EF1_32 = { EF1,THIRTY_SECOND};
  Note  E1_32 = { E1,THIRTY_SECOND};
  Note  F1_32 = { F1,THIRTY_SECOND};
  Note  GF1_32 = { GF1,THIRTY_SECOND};
  Note  G1_32 = { G1,THIRTY_SECOND};
  Note  AF1_32 = { AF1,THIRTY_SECOND};
  Note  A1_32 = { A1,THIRTY_SECOND};
  Note  BF1_32 = { BF1,THIRTY_SECOND};
  Note  B1_32 = { B1,THIRTY_SECOND};
  Note C2_32 = {C2,THIRTY_SECOND};
  Note  DF2_32 = { DF2,THIRTY_SECOND};
  Note  D2_32 = { D2,THIRTY_SECOND};
  Note  EF2_32 = { EF2,THIRTY_SECOND};
  Note  E2_32 = { E2,THIRTY_SECOND};
  Note  F2_32 = { F2,THIRTY_SECOND};
  Note  GF2_32 = { GF2,THIRTY_SECOND};
  Note  G2_32 = { G2,THIRTY_SECOND};
  Note  AF2_32 = { AF2,THIRTY_SECOND};
  Note  A2_32 = { A2,THIRTY_SECOND};
  Note  BF2_32 = { BF2,THIRTY_SECOND};
  Note  B2_32 = { B2,THIRTY_SECOND};
  Note C3_32 = {C3,THIRTY_SECOND};
  Note  DF3_32 = { DF3,THIRTY_SECOND};
  Note  D3_32 = { D3,THIRTY_SECOND};
  Note  EF3_32 = { EF3,THIRTY_SECOND};
  Note  E3_32 = { E3,THIRTY_SECOND};
  Note  F3_32 = { F3,THIRTY_SECOND};
  Note  GF3_32 = { GF3,THIRTY_SECOND};
  Note  G3_32 = { G3,THIRTY_SECOND};
  Note  AF3_32 = { AF3,THIRTY_SECOND};
  Note  A3_32 = { A3,THIRTY_SECOND};
  Note  BF3_32 = { BF3,THIRTY_SECOND};
  Note  B3_32 = { B3,THIRTY_SECOND};
  Note C4_32 = {C4,THIRTY_SECOND};
  Note  DF4_32 = { DF4,THIRTY_SECOND};
  Note  D4_32 = { D4,THIRTY_SECOND};
  Note  EF4_32 = { EF4,THIRTY_SECOND};
  Note  E4_32 = { E4,THIRTY_SECOND};
  Note  F4_32 = { F4,THIRTY_SECOND};
  Note  GF4_32 = { GF4,THIRTY_SECOND};
  Note  G4_32 = { G4,THIRTY_SECOND};
  Note  AF4_32 = { AF4,THIRTY_SECOND};
  Note  A4_32 = { A4,THIRTY_SECOND};
  Note  BF4_32 = { BF4,THIRTY_SECOND};
  Note  B4_32 = { B4,THIRTY_SECOND};
  Note C5_32 = {C5,THIRTY_SECOND};
  Note  DF5_32 = { DF5,THIRTY_SECOND};
  Note  D5_32 = { D5,THIRTY_SECOND};
  Note  EF5_32 = { EF5,THIRTY_SECOND};
  Note  E5_32 = { E5,THIRTY_SECOND};
  Note  F5_32 = { F5,THIRTY_SECOND};
  Note  GF5_32 = { GF5,THIRTY_SECOND};
  Note  G5_32 = { G5,THIRTY_SECOND};
  Note  AF5_32 = { AF5,THIRTY_SECOND};
  Note  A5_32 = { A5,THIRTY_SECOND};
  Note  BF5_32 = { BF5,THIRTY_SECOND};
  Note  B5_32 = { B5,THIRTY_SECOND};
  Note REST_32 = {REST,THIRTY_SECOND};
  Note C1_64 = {C1,SIXTY_FOURTH};
  Note  DF1_64 = { DF1,SIXTY_FOURTH};
  Note  D1_64 = { D1,SIXTY_FOURTH};
  Note  EF1_64 = { EF1,SIXTY_FOURTH};
  Note  E1_64 = { E1,SIXTY_FOURTH};
  Note  F1_64 = { F1,SIXTY_FOURTH};
  Note  GF1_64 = { GF1,SIXTY_FOURTH};
  Note  G1_64 = { G1,SIXTY_FOURTH};
  Note  AF1_64 = { AF1,SIXTY_FOURTH};
  Note  A1_64 = { A1,SIXTY_FOURTH};
  Note  BF1_64 = { BF1,SIXTY_FOURTH};
  Note  B1_64 = { B1,SIXTY_FOURTH};
  Note C2_64 = {C2,SIXTY_FOURTH};
  Note  DF2_64 = { DF2,SIXTY_FOURTH};
  Note  D2_64 = { D2,SIXTY_FOURTH};
  Note  EF2_64 = { EF2,SIXTY_FOURTH};
  Note  E2_64 = { E2,SIXTY_FOURTH};
  Note  F2_64 = { F2,SIXTY_FOURTH};
  Note  GF2_64 = { GF2,SIXTY_FOURTH};
  Note  G2_64 = { G2,SIXTY_FOURTH};
  Note  AF2_64 = { AF2,SIXTY_FOURTH};
  Note  A2_64 = { A2,SIXTY_FOURTH};
  Note  BF2_64 = { BF2,SIXTY_FOURTH};
  Note  B2_64 = { B2,SIXTY_FOURTH};
  Note C3_64 = {C3,SIXTY_FOURTH};
  Note  DF3_64 = { DF3,SIXTY_FOURTH};
  Note  D3_64 = { D3,SIXTY_FOURTH};
  Note  EF3_64 = { EF3,SIXTY_FOURTH};
  Note  E3_64 = { E3,SIXTY_FOURTH};
  Note  F3_64 = { F3,SIXTY_FOURTH};
  Note  GF3_64 = { GF3,SIXTY_FOURTH};
  Note  G3_64 = { G3,SIXTY_FOURTH};
  Note  AF3_64 = { AF3,SIXTY_FOURTH};
  Note  A3_64 = { A3,SIXTY_FOURTH};
  Note  BF3_64 = { BF3,SIXTY_FOURTH};
  Note  B3_64 = { B3,SIXTY_FOURTH};
  Note C4_64 = {C4,SIXTY_FOURTH};
  Note  DF4_64 = { DF4,SIXTY_FOURTH};
  Note  D4_64 = { D4,SIXTY_FOURTH};
  Note  EF4_64 = { EF4,SIXTY_FOURTH};
  Note  E4_64 = { E4,SIXTY_FOURTH};
  Note  F4_64 = { F4,SIXTY_FOURTH};
  Note  GF4_64 = { GF4,SIXTY_FOURTH};
  Note  G4_64 = { G4,SIXTY_FOURTH};
  Note  AF4_64 = { AF4,SIXTY_FOURTH};
  Note  A4_64 = { A4,SIXTY_FOURTH};
  Note  BF4_64 = { BF4,SIXTY_FOURTH};
  Note  B4_64 = { B4,SIXTY_FOURTH};
  Note C5_64 = {C5,SIXTY_FOURTH};
  Note  DF5_64 = { DF5,SIXTY_FOURTH};
  Note  D5_64 = { D5,SIXTY_FOURTH};
  Note  EF5_64 = { EF5,SIXTY_FOURTH};
  Note  E5_64 = { E5,SIXTY_FOURTH};
  Note  F5_64 = { F5,SIXTY_FOURTH};
  Note  GF5_64 = { GF5,SIXTY_FOURTH};
  Note  G5_64 = { G5,SIXTY_FOURTH};
  Note  AF5_64 = { AF5,SIXTY_FOURTH};
  Note  A5_64 = { A5,SIXTY_FOURTH};
  Note  BF5_64 = { BF5,SIXTY_FOURTH};
  Note  B5_64 = { B5,SIXTY_FOURTH};
  Note REST_64 = {REST,SIXTY_FOURTH};
	
	/*
	Song New_Bark_Town = {NBT_ID, "New Bark Town", BPM60, {D4_8, E4_8, GF4_4, A4_4, G4_8, GF4_8, E4_8, G4_8, GF4_4dot, D4_8, A3_4, REST_8, G3_16, A3_16, 
			B3_4, D4_4, E4_8, D4_8, DF4_8, D4_8, E4_4dot, GF4_8, E4_4, REST_8, D4_16, E4_16, GF4_4, A4_4, BF4_8, A4_8, G4_8, BF4_8, A4_4dot, DF5_8, D5_4, REST_8,
			E4_16, GF4_16, G4_4dot, A4_8, B4_2, A4_4dot, G4_16, GF4_16, //4 */
			/* Beginning of part jump */	
	/*A2_16, DF3_16, E3_16, G3_16, A3_16, DF4_16, E4_16, A4_16, B4_4dot, A4_4dot, G4_4, E4_1, B4_4dot, A4_4dot, D5_4, DF5_1,
			B4_4dot, A4_4dot, G4_4, E4_1, B4_4dot, A4_4dot, D5_4, E5_1},
		, {REST_4}, 3};
		*/
	Song testSong1 =  {0, "TestSong1", BPM60, {REST_8, D5_4}, 2, {D5_4, D5_4}, 2};
	testSongs.songs[0] = testSong1;

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
