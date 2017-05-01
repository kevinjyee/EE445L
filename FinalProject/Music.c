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
#include <stdlib.h>
#include "../inc/tm4c123gh6pm.h"
#include "SysTick.h"
#include "Timer0A.h"
#include "Timer1.h"
#include "Timer3.h"
#include "Music.h"
#include "DAC.h"
#include "Sound.h"
#include "Heap.h"
#include "Globals.h"
#define WAVETABLE_LENGTH					64
#define	NUM_TEMPOS								10
#define EnvOnOFF									0
#define MAX_VOLUME								7

#define zero 0
#define three 3

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
uint32_t StartCritical (void);// previous I bit, disable interrupts
void EndCritical(uint32_t sr);// restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint8_t uninitialized = 1; // Tells initialization code if songs have been instantiated.

volatile uint8_t I = 0; // Index for soprano note waveform.
volatile uint8_t J = 0; // Index for alto note waveform.
volatile uint8_t M = 0; // Index for third note waveform.
volatile uint8_t N = 0; // Index for fourth note waveform.
volatile uint8_t E = 0; // Index for soprano note envelope.
volatile uint8_t K = 0; // Index for alto note envelope.
volatile uint8_t current_Soprano_Beats = 0; // Counter for current soprano note's number of beats (in 1/128th note resolution).
volatile uint8_t current_Alto_Beats = 0; // Counter for current alto note's number of beats (in 1/128th note resolution).
volatile uint8_t current_Third_Beats = 0;
volatile uint8_t current_Fourth_Beats = 0;
volatile uint16_t soprano_Note_Index = 0; // Index for current soprano note. Moves independently of alto index.
volatile uint16_t alto_Note_Index = 0; // Index for current alto note. Moves independently of soprano index.
volatile uint8_t letOff = 0;						// Should be high when need to leave space between notes.		
volatile uint8_t enableEnv = 0; // Enable note enveloping.
volatile uint8_t scalar1 =1; // Scalar used for soprano envelope calculation.
volatile uint8_t scalar2 =1; // Scalar used for alto enveloping.

//extern volatile int currentSongPos; // Global for position in current song list.
//extern volatile int lastSongPos; // GLobal for last song playing.
//extern volatile int currentMode; // Global for waveform type.

//extern volatile int Playing;
volatile int Playing = 0;
//extern volatile int songVolume;

  

// Enum for note pitches. Will correspond to reload values.
enum pitch{
	C1, DF1, D1, EF1, E1, F1, GF1, G1, AF1, A1, BF1, B1,
	C2, DF2, D2, EF2, E2, F2, GF2, G2, AF2, A2, BF2, B2,
	C3, DF3, D3, EF3, E3, F3, GF3, G3, AF3, A3, BF3, B3,
	C4, DF4, D4, EF4, E4, F4, GF4, G4, AF4, A4, BF4, B4,
	C5, DF5, D5, EF5, E5, F5, GF5, G5, AF5, A5, BF5, B5,
	REST, NUM_NOTES
};

// Reload values for different pitches.
const unsigned short SineUpdateDelay[NUM_NOTES] = {
	11945,11274,10641,10044,9480,8948,8446,7972,7525,7102,6704,
	6327,5972,5637,5321,5022,4740,4474,4223,3986,3762,3551,3352,
	3164,2986,2819,2660,2511,2370,2237,2112,1993,1881,1776,1676,1582,
	1493,1409,1330,1256,1185,1119,1056,997,941,888,838,791,747,705,665,
	628,593,559,528,498,470,444,419,395,0
};

// Waveform type enum.
enum mode{
	SINE, FLUTE, BRASS
};

// Note length enum.
enum time_value{
	ONE_HUNDRED_TWENTY_EIGHTH, SIXTY_FOURTH, THIRTY_SECOND, SIXTEENTH, EIGHTH, EIGHTH_DOT, QUARTER, QUARTER_DOT, HALF, HALF_DOT, WHOLE, NUM_DIFF_NOTE_DURATIONS
};

// Note length values.
const uint8_t time_value[NUM_DIFF_NOTE_DURATIONS] = {
	1, 2, 4, 8, 16, 24, 32, 48, 64, 96, 128
};

// Tempo selection enum.
enum tempo{
	BPM60, BPM70, BPM80, BPM90, BPM100, BPM110, BPM120, BPM130, BPM160, BPM240
};

// Reload multiplier for tempo setting.
const uint32_t tempo_Reload[NUM_TEMPOS] = {
	60, 70, 80, 90, 100, 110, 120, 130,160,240
};

// Note structure: will have pitch and note duration.
typedef struct
{
	enum pitch note_pitch;
	enum time_value note_time_value;
} Note;

typedef struct _NoteNode NoteNode;
struct _NoteNode
{
	Note* note;
	NoteNode* next;
};

// Song structure: has a unique id, a name, a tempo, soprano and alto notes, and an enable envelope boolean.
typedef struct
{
	int id;								// For use in menu selection
	char name[20];					// For use in printing name to LCD.
	enum tempo my_tempo;						// Sets tempo of song
  NoteNode* soprano_Notes;  // Soprano melody for song.
	NoteNode* first_Soprano_Note;
	//int num_Soprano_Notes;		// Number of soprano notes.
	NoteNode* alto_Notes;			// Alto melody for song.
	NoteNode* first_Alto_Note;
	NoteNode* third_Notes;
	NoteNode* first_Third_Note;
	NoteNode* fourth_Notes;
	NoteNode* first_Fourth_Notes;
	//Note alto_Notes[80];			// Alto melody for song.
	//int num_Alto_Notes;				// Number of alto notes.
	int enable_envelope;
} Song;  /* Song structs */

typedef struct
{
	Song songs[3];
} Song_Choices; /* Playlist for Lab 5! */

// Below we declared notes which we use in our songs.
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
  Note C4_1 = {C4,WHOLE};
  Note  DF4_1 = { DF4,WHOLE};
  Note  D4_1 = { D4,WHOLE};
  Note  EF4_1 = { EF4,WHOLE};
  Note  F4_1 = { F4,WHOLE};
  Note  GF4_1 = { GF4,WHOLE};
  Note  G4_1 = { G4,WHOLE};
  Note  AF4_1 = { AF4,WHOLE};
  Note  A4_1 = { A4,WHOLE};
  Note  BF4_1 = { BF4,WHOLE};
  Note  B4_1 = { B4,WHOLE};
	Note C5_1 = {C5,WHOLE};
	Note REST_1 = {REST,WHOLE};
	Note  F4_8 = { F4,EIGHTH};
  Note  B4_4 = { B4,QUARTER};
  Note  B4_8 = { B4,EIGHTH};
  Note  E5_8 = { E5,EIGHTH};
  Note C5_8 = {C5,EIGHTH};
  Note  E4_2 = { E4,HALF};

  Note  G4_2 = { G4,HALF};
	Note  F3_8 = { F3,EIGHTH};
  Note C4_8 = {C4,EIGHTH};
	Note  F2_8 = { F2,EIGHTH};
	Note  E2_8 = { E2,EIGHTH};
	Note C2_8 = {C2,EIGHTH};
	
	Note GF3_8 = {GF3, EIGHTH};
	Note GF3_16 = {GF3, SIXTEENTH};
	Note D3_16 = {D3, SIXTEENTH};
	Note B2_16 = {B2, SIXTEENTH};
	Note B3_8 = {B3, EIGHTH};
	Note G1_1 = {G1, WHOLE};
	Note A1_1 = {A1, WHOLE};
	Note A1_2 = {A1, HALF};
	Note REST_4dot = {REST, QUARTER_DOT};
	Note REST_16 = {REST, SIXTEENTH};
	
	Note BF3_4 = {BF3, QUARTER};
	Note EF4_8 = {EF4, EIGHTH};
	Note C4_4 = {C4, QUARTER};
	Note G3_1 = {G3, WHOLE};
	Note REST_2dot = {REST, HALF_DOT};
	Note G3_2dot = {G3, HALF_DOT};
	Note F3_2dot = {F3, HALF_DOT};
	Note EF3_2dot = {EF3, HALF_DOT};
	Note D3_2dot = {D3, HALF_DOT};
	Note REST_2 = {REST, HALF};
	Note EF3_8 = {EF3, EIGHTH};
	Note GF2_2dot = {GF2, HALF_DOT};
	Note E3_4 = {E3, QUARTER};
	Note C3_2dot = {C3, HALF_DOT};
	Note G2_2dot = {G2, HALF_DOT};
	Note E3_2dot = {E3, HALF_DOT};
	Note EF3_4 = {EF3, QUARTER};
	Note D3_4 = {D3, QUARTER};
	Note BF2_2dot = {BF2, HALF_DOT};
	Note D2_2dot = {D2, HALF_DOT};
	Note A2_2 = {A2, HALF};
	Note F2_2dot = {F2, HALF_DOT};
	Note EF2_2dot = {EF2, HALF_DOT};
	Note C2_2dot = {C2, HALF_DOT};
	Note G1_2dot = {G1, HALF_DOT};
	
	
Note* LWSoprano[44] = { &F4_8, &A4_8, &B4_4, &F4_8, &A4_8, &B4_4, &F4_8, &A4_8, &B4_8, &E5_8, &D5_4, &B4_8, &C5_8, &B4_8, &G4_8,
			&E4_2, &REST_8, &D3_8, &E4_8, &G4_8, &E4_2, &REST_4, &F4_8, &A4_8, &B4_4, &F4_8, &A4_8, &B4_4, &F4_8, &A4_8, &B4_8, &E5_8,
			&D5_4, &B4_8, &C5_8, &E5_8, &B4_8, &G4_2, &REST_8, &B4_8, &G4_8, &D3_8, &E4_2, &REST_4
};
Note* LWAlto[64] = {&F2_8, &C3_8, &C3_8, &C3_8, &F2_8, &C3_8, &C3_8, &C3_8, &F2_8, &C3_8, &C3_8, &C3_8, &F2_8, &C3_8, &C3_8,
			&C3_8, &E2_8, &C3_8, &C3_8, &C3_8, &E2_8, &C3_8, &C3_8, &C2_8, &E2_8, &C3_8, &C3_8, &C3_8, &E2_8, &C3_8, &C3_8, &C3_8,
			&F2_8, &C3_8, &C2_8, &C3_8, &F2_8, &C3_8, &C3_8, &C3_8, &F2_8, &C3_8, &C3_8, &C3_8, &F2_8, &C3_8, &C3_8, &C3_8, &E2_8,
			&C3_8, &C3_8, &C3_8, &E2_8, &C3_8, &C3_8, &C2_8, &E2_8, &C3_8, &C3_8, &C3_8, &E2_8, &C3_8, &C3_8, &C2_8
};
Note* NBTSoprano[68] = {
	&D4_8, &E4_8, &GF4_4, &A4_4, &G4_8, &GF4_8, &E4_8, &G4_8, &GF4_4dot, &D4_8, &A3_4, &REST_8, &G3_16,
	&A3_16, &	B3_4, &D4_4, &E4_8, &D4_8, &DF4_8, &D4_8, &E4_4dot, &GF4_8, &E4_4, &REST_8, &D4_16, &E4_16, &GF4_4, &A4_4,
	&BF4_8, &A4_8, &G4_8, &BF4_8, &	A4_4dot, &DF5_8, &D5_4, &REST_8, &E4_16, &GF4_16, &G4_4dot, &A4_8, &B4_2, &A4_4dot,
	&G4_16, &GF4_16, &A2_16, &DF3_16, &E3_16, &G3_16, &A3_16, &	DF4_16, &E4_16, &A4_16, &B4_4dot, &A4_4dot, &G4_4, &E4_1,
	&B4_4dot, &A4_4dot, &D5_4, &DF5_1, &B4_4dot, &A4_4dot, &G4_4, &E4_1, &B4_4dot, &A4_4dot, &D5_4, &E5_2dot
};
Note* NBTAlto[103] = {
	&REST_4, &D2_4, &REST_8, &D2_16, &D2_16, &D2_8, &D2_4, &BF3_8, &D2_4, &REST_8, &D2_16, &D2_16, &D2_8,
	&D2_4, &C3_8, &G2_4, &REST_8, &G2_16, &G2_16, &AF2_8, &AF2_4, &D2_8, &A3_4, &REST_8, &A3_16, &A3_16, &A2_8, &A2_4, &DF2_8,
	&D2_4, &REST_8, &D2_16, &D2_16, &D2_8, &D2_4, &BF3_8, &D2_4, &REST_8, &D2_16, &D2_16, &D2_8, &D2_4, &C3_8, &G2_4, &REST_8,
	&G2_16, &G2_16, &AF1_8, &AF1_4, &D2_8, &A2_4, &REST_8, &A2_16, &A2_16, &A1_8, &A1_4dot, &G2_8, &B2_8, &D3_8, &GF3_4, &G3_4,
	&D3_8, &A3_8, &DF3_8, &E3_8, &G3_4, &A3_4, &E3_8, &G2_8, &B2_8, &D3_8, &GF3_4, &G3_4, &D3_8, &A3_8, &DF3_8, &E3_8, &G3_4,
	&A3_4, &E3_8, &G2_8, &B2_8, &D3_8, &GF3_4, &G3_4, &D3_8, &A3_8, &DF3_8, &E3_8, &G3_4, &A3_4, &E3_8, &G2_8, &B2_8, &D3_8,
  &GF3_4, &G3_4, &D3_8, &A3_8, &DF3_8, &E3_8, &G3_4, &A3_8
};
Note* NBTThird[77] = {
	&REST_4, &GF3_8, &D3_8, &A3_8, &D3_8, &GF3_8, &D3_8, &BF3_8, &D3_8, &GF3_8, &D3_8, &B3_8, &D3_8, &GF3_8, &D3_8, &C4_8, &G3_8, 
	&B2_8, &G2_8, &G3_8, &B2_8, &B2_8, &AF2_8, &E3_8, &B2_8, &DF3_8, &A2_8, &G3_8, &GF3_8, &E3_16, &GF3_16, &E3_16, &D3_16, &DF3_16, &B2_16,
	&A2_16, &G2_16, &GF3_8, &D3_8, &A3_8, &D3_8, &GF3_8, &D3_8, &BF3_8, &D3_8, &GF3_8, &D3_8, &B3_8, &D3_8, &GF3_8, &D3_8, &C4_8, &G3_8, 
	&B2_8, &G2_8, &G3_8, &B2_8, &B2_8, &AF2_8, &E3_8, &B2_8, &DF3_8, &A2_8, &E3_8, &DF3_8, &A1_8, &A1_4, &DF2_8, &G1_1, &A1_1, &G1_1,
	&A1_1, &G1_1, &A1_1, &G1_1, &A1_2, &REST_4
};

Note* CotBSoprano[135] = {
	&BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4,
	&BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4,
	&BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4,
	&D4_4, &C4_8, &D4_8, &BF3_4, &D4_4, &C4_8, &D4_8, &BF3_4, &D4_4, &C4_8, &D4_8, &BF3_4, &D4_4, &C4_8, &D4_8, &BF3_4,
	&G4_4, &G4_8, &G4_8, &F4_8, &EF4_8, &D4_4, &D4_8, &D4_8, &C4_8, &BF3_8, &C4_4, &C4_8, &C4_8, &D4_8, &C4_8, 
	&BF3_4, &A3_8, &BF3_8, &G3_4, &D3_8, &E3_8, &GF3_8, &G3_8, &A3_8, &BF3_8, &C4_8, &D4_8, &C4_4, &BF3_4,
	&D3_8, &E3_8, &GF3_8, &G3_8, &A3_8, &BF3_8, &C4_8, &D4_8, &C4_4, &BF3_4,
	&BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4,
	&BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_4, &BF3_4, &A3_8, &BF3_8, &G3_1
};

Note* CotBAlto[53] = {
	&REST_2dot, &REST_2dot, &REST_2dot, &REST_2dot, &G3_2dot, &F3_2dot, &EF3_2dot, &D3_2dot,
	&G3_4, &REST_2, &G3_4, &REST_2, &G3_4, &REST_2, &G3_4, &REST_2,
	&BF3_4, &REST_4, &G3_4, &BF3_4, &REST_4, &G3_4, &BF3_4, &REST_4, &G3_4, &BF3_4, &REST_4, &G3_4, 
	&G3_4, &G3_8, &G3_8, &F3_8, &EF3_8, &D3_2dot, &EF3_2dot, &D3_2dot, &GF2_2dot,
	&E3_4, &GF3_4, &G3_4, &GF2_2dot, &E3_4, &GF3_4, &G3_4, &D3_2dot, &C3_2dot, &F3_2dot, &EF3_2dot, 
	&D3_2dot, &D3_2dot, &G2_2dot, &G2_2dot, &G2_2dot
};

Note* CotBThird[37] = {
	&G3_2dot, &F3_2dot, &EF3_2dot, &D3_2dot, &EF3_2dot, &D3_2dot, &C3_2dot, &G2_2dot, &C3_2dot, &D3_2dot,
	&C3_2dot, &D3_2dot, &D3_2dot, &E3_2dot, &F3_2dot, &E3_4, &EF3_4, &D3_4, &D3_2dot, &BF2_2dot, &BF2_2dot, &BF2_2dot,
	&D2_2dot, &A2_2, &G2_4, &D2_2dot, &A2_2, &G2_4, &F2_2dot, &EF2_2dot, &D2_2dot, &C2_2dot, &G1_2dot, &G1_2dot,
	&G1_2dot, &G1_2dot, &G1_2dot
};
	
Song_Choices testSongs;

// 1024-amplitude sine-wave.
const uint16_t Sine_Wave[WAVETABLE_LENGTH] = {
	1024, 1124, 1224, 1321, 1416, 1507, 1593, 1674, 1748, 1816, 1875, 1927, 1970,
	2004, 2028, 2043, 2047, 2043, 2028, 2004, 1970, 1927, 1875, 1816, 1748, 1674,
	1593, 1507, 1416, 1321, 1224, 1124, 1024, 924, 824, 727, 632, 541, 455, 374,
	300, 232, 173, 121, 78, 44, 20, 5, 0, 5, 20, 44, 78, 121, 173, 232, 300, 374,
	455, 541, 632, 727, 824, 924
};

const uint16_t Sine_Wave_10_bit[WAVETABLE_LENGTH] = {  
  512,562,612,661,708,753,796,837,874,908,938,
  964,985,1002,1014,1022,1024,1022,1014,1002,985,964,
  938,908,874,837,796,753,708,661,612,562,512,462,
  412,363,316,271,228,187,150,116,86,60,
  39,22,10,2,0,2,10,22,39,60,
  86,116,150,187,228,271,316,363,412,462
};

const uint16_t Sine_Wave_7_bit[WAVETABLE_LENGTH] = {
	  64,70,76,83,88,94,100,105,109,113,117,
  120,123,125,127,128,128,128,127,125,123,120,
  117,113,109,105,100,94,88,83,76,70,64,58,
  52,45,40,34,28,23,19,15,11,8,
  5,3,1,0,0,0,1,3,5,8,
  11,15,19,23,28,34,40,45,52,58

};


// 11-bit flute wave
const uint16_t Flute_Wave[WAVETABLE_LENGTH] = {
  1007,1252,1374,1548,1698,1797,1825,1797,1675,1562,1383,
  1219,1092,1007,913,890,833,847,810,777,744,674,
  598,551,509,476,495,533,589,659,758,876,
	1007,1252,1374,1548,1698,1797,1825,1797,1675,1562,1383,
  1219,1092,1007,913,890,833,847,810,777,744,674,
  598,551,509,476,495,533,589,659,758,876
};  

const uint16_t Flute_Wave_10_bit[WAVETABLE_LENGTH] = {
	  508,570,602,646,685,710,717,710,679,650,604,
  562,529,508,484,478,463,467,457,449,440,422,
  403,391,380,372,377,386,401,419,444,474,508,570,
  602,646,685,710,717,710,679,650,604,562,
  529,508,484,478,463,467,457,449,440,422,
  403,391,380,372,377,386,401,419,444,474
};

// 11-BIT TRUMPET
const uint16_t Trumpet_Wave[WAVETABLE_LENGTH] = {
  1024,1024,1014,1008,1022,1065,1093,1006,858,711,612,
  596,672,806,952,1074,1154,1191,1202,1216,1236,1255,
  1272,1302,1318,1299,1238,1140,1022,910,827,779,
  758,757,782,856,972,1088,1177,1226,1232,1203,
  1157,1110,1067,1028,993,958,929,905,892,900,
  940,1022,1125,1157,1087,965,836,783
};

const uint16_t Trumpet_Wave_10_bit[WAVETABLE_LENGTH] = {
	  545,561,593,727,1070,1130,727,283,202,202,238,
  262,295,359,525,605,670,775,888,957,856,993,
  807,565,242,81,20,57,121,222,323,484,545,561,
  593,727,1070,1130,727,283,202,202,238,262,
  295,359,525,605,670,775,888,957,856,993,
  807,565,242,81,20,57,121,222,323,484

};

// Envelope scalars.
const uint16_t envelopes[128] = { 
	8,32,64,96,128,127,125,123,121,119,118,117,116,
	115,114,113,112,111,110,109,108,107,106,105,104,
	103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,
	88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,
	70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,
	51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,
	30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,
  9,8,7,6,5,4,3,2,1,};

// Used to output a waveform. Called by soprano and alto wave generation function.
//		Waveform type controlled by currentMode.
void Output_Wave(){
	if(EnvOnOFF){ // With enveloping.
		switch(SINE){
			case SINE:
					DAC_Out(((Sine_Wave[I]*envelopes[E]/128 + Sine_Wave[J])*envelopes[K]/128));
				break;
			case FLUTE:
					DAC_Out(((Flute_Wave[I]*envelopes[E]/128 + Flute_Wave[J])*envelopes[K]/128));
				break;
			case BRASS:
					DAC_Out(((Trumpet_Wave[I]*envelopes[E]/128 + Trumpet_Wave[J])*envelopes[K]/128));
				break;
		}
	}
	else{
		switch(SINE){ // Without enveloping.
			case SINE:
					DAC_Out((((Sine_Wave_10_bit[I] + Sine_Wave_10_bit[J] + Sine_Wave_10_bit[M]) * Volume) / MAX_VOLUME) * Fade / MAX_FADE);
				break;
			case FLUTE:
					DAC_Out((((Flute_Wave_10_bit[I] + Flute_Wave_10_bit[J] + Flute_Wave_10_bit[M]) * Volume) / MAX_VOLUME) * Fade / MAX_FADE);
				break;
			case BRASS:
					DAC_Out((((Trumpet_Wave_10_bit[I] + Trumpet_Wave_10_bit[J] + Trumpet_Wave_10_bit[M]) * Volume) / MAX_VOLUME) * Fade / MAX_FADE);
				break;
		}
	}
}
	
// Output soprano waveform.
void OutputSopranoWave(void){
	  //DAC_Out(Sine_Wave[I] + Sine_Wave[J]);
	uint32_t sr = StartCritical();
	Output_Wave();
	I = (I + 1) % WAVETABLE_LENGTH; // Modify soprano note index.
	
	EndCritical(sr);
}

// Output alto waveform.
void OutputAltoWave(void){
	//DAC_Out(Sine_Wave[I] + Sine_Wave[J]);
	uint32_t sr = StartCritical();
	Output_Wave();
	J = (J + 1) % WAVETABLE_LENGTH; // Modify alto note index.
	EndCritical(sr);
}

// Output soprano waveform.
void OutputThirdWave(void){
	  //DAC_Out(Sine_Wave[I] + Sine_Wave[J]);
	uint32_t sr = StartCritical();
	Output_Wave();
	M = (M + 1) % WAVETABLE_LENGTH; // Modify soprano note index.
	
	EndCritical(sr);
}

Note* current_Soprano_Note;
Note* current_Alto_Note;
Note* current_Third_Note;

// ***************** Play_Song *****************
void Play_Song(void){
	Song* current_Song = &testSongs.songs[2];
	enableEnv = 0;
	uint32_t sr;

	current_Soprano_Note = current_Song->soprano_Notes->note; // Get current soprano note.
	if(current_Soprano_Beats == 0){ // If at a new note, load Timer0A with new soprano frequency.
		sr = StartCritical();
		E = 0;
		EndCritical(sr);
		Timer0A_Init(&OutputSopranoWave, SineUpdateDelay[current_Soprano_Note->note_pitch]);
	} else if(current_Soprano_Beats == (time_value[current_Soprano_Note->note_time_value] - 1)){ // Update note index if moving to next note.
		current_Song->soprano_Notes = current_Song->soprano_Notes->next;
	} else if(current_Soprano_Beats == (time_value[current_Soprano_Note->note_time_value] - 2)){
		Timer0A_Init(&OutputSopranoWave, SineUpdateDelay[REST]);
	}
	current_Soprano_Beats = (current_Soprano_Beats + 1) % time_value[current_Soprano_Note->note_time_value]; // Update beats
	
	current_Alto_Note = current_Song->alto_Notes->note;
	if(current_Alto_Beats == 0){ // If at new note, load Timer1A with new alto frequency.
		sr = StartCritical();
		K=0;
		EndCritical(sr);
		Timer1_Init(&OutputAltoWave, SineUpdateDelay[current_Alto_Note->note_pitch]);
	} else if(current_Alto_Beats == (time_value[current_Alto_Note->note_time_value] - 1)){ // Update note index if moving to next note.
		current_Song->alto_Notes = current_Song->alto_Notes->next;
	} else if(current_Alto_Beats == (time_value[current_Alto_Note->note_time_value] - 2)){
		Timer1_Init(&OutputAltoWave, SineUpdateDelay[REST]);
	}
	current_Alto_Beats = (current_Alto_Beats + 1) % time_value[current_Alto_Note->note_time_value];
	
	current_Third_Note = current_Song->third_Notes->note; // Get current third note.
	if(current_Third_Beats == 0){ // If at a new note, load Timer0A with new soprano frequency.
		sr = StartCritical();
		E = 0;
		EndCritical(sr);
		Timer3_Init(&OutputThirdWave, SineUpdateDelay[current_Third_Note->note_pitch]);
	} else if(current_Third_Beats == (time_value[current_Third_Note->note_time_value] - 1)){ // Update note index if moving to next note.
		current_Song->third_Notes = current_Song->third_Notes->next;
	} else if(current_Third_Beats == (time_value[current_Third_Note->note_time_value] - 2)){
		Timer3_Init(&OutputThirdWave, SineUpdateDelay[REST]);
	}
	current_Third_Beats = (current_Third_Beats + 1) % time_value[current_Third_Note->note_time_value]; // Update beats
	
	E = (E+(1*(128/time_value[current_Soprano_Note->note_time_value]))) % 128;
	K = (K+(1*(128/time_value[current_Alto_Note->note_time_value]))) % 128;
	
}

// Creates a circular linked list of NoteNodes.
NoteNode* create_LinkedList_From_Array(Note* notes[], uint32_t length){
	
		NoteNode* current = NULL;
		NoteNode* next = NULL;
		NoteNode* head = NULL;
		for(int i = 0; i < length; i++){
			NoteNode* next = (NoteNode*) Heap_Malloc(sizeof(NoteNode));
			next->note = notes[i];
			next->next = NULL;
			if(current != NULL){
				current->next = next;
			}
			current = next;
			if(i == 0){
				head = current;
			} else if(i == length - 1){
				current->next = head; // Note arrays are circular linked lists.
			}
		}
		return head;

	return NULL;
}


		Song testSong1;
		Song testSong2;
		Song New_Bark_Town;
// ***************** Music_Init ****************
// Creates our song selection.
void Music_Init(void){
	
				
	
		NoteNode* Lost_Woods_Soprano_Notes = create_LinkedList_From_Array((Note**) LWSoprano, 44);
		NoteNode* Lost_Woods_Alto_Notes = create_LinkedList_From_Array((Note**) LWAlto, 64);
		Song Lost_Woods = {0, "Lost Woods", BPM60, Lost_Woods_Soprano_Notes, Lost_Woods_Soprano_Notes, Lost_Woods_Alto_Notes, Lost_Woods_Alto_Notes,Lost_Woods_Alto_Notes,Lost_Woods_Alto_Notes,NULL,NULL, 0};
	
		NoteNode* NBT_Soprano_Notes = create_LinkedList_From_Array((Note**) NBTSoprano, 68);
		NoteNode* NBT_Alto_Notes = create_LinkedList_From_Array((Note**) NBTAlto, 103);
		NoteNode* NBT_Third_Notes = create_LinkedList_From_Array((Note**) NBTThird, 77);
		Song New_Bark_Town = {1, "New Bark Town", BPM60, NBT_Soprano_Notes, NBT_Soprano_Notes, NBT_Alto_Notes, NBT_Alto_Notes, NBT_Third_Notes, NBT_Third_Notes, NULL, NULL, 0};
				
		NoteNode* CotB_Soprano_Notes = create_LinkedList_From_Array((Note**) CotBSoprano, 135);
		NoteNode* CotB_Alto_Notes = create_LinkedList_From_Array((Note**) CotBAlto, 53);
		NoteNode* CotB_Third_Notes = create_LinkedList_From_Array((Note**) CotBThird, 37);
		Song Carol_of_the_Bells = {2, "Carol of the Bells", BPM60, CotB_Soprano_Notes, CotB_Soprano_Notes, CotB_Alto_Notes, CotB_Alto_Notes, CotB_Third_Notes, CotB_Third_Notes, NULL, NULL, 0};
				
		
		testSongs.songs[0] = Lost_Woods;
		testSongs.songs[1] = New_Bark_Town;
		testSongs.songs[2] = Carol_of_the_Bells;
		
		

}
// ***************** Play ****************
// Plays music when called.
// Inputs:  none
// Outputs: none
void Play(void){
	if(uninitialized){ // Initialize music if not dones.
		
		Music_Init();
		uninitialized = 0;
	}
	SysTick_Init(&Play_Song, Current_Tempo); // Initialize tempo counter.
	Playing = 1;
	
}

// ***************** Pause ****************
// Pauses music when called.
// Inputs:  none
// Outputs: none
void Pause(void){
	Playing = 0;
	SysTick_Halt();
	Timer0A_Halt();
	Timer1A_Halt();
	Timer3A_Halt();
}

// ***************** change_Song ****************
// Changes the song that is playing or begins playing the selected song.
// Inputs:  none
// Outputs: none
void Change_Song(){
	if(uninitialized){ // If uninitialized, initialize music.
		
		Music_Init();
		uninitialized = 0;
	}
	Rewind();
}

// ***************** Mess_With_Tempo ****************
// Changes the tempo, initializing if music hasn't been.
// Inputs:  none
// Outputs: none
void Mess_With_Tempo(uint16_t current_tempo){
	if(uninitialized){ // If uninitialized, initialize music.
		
		Music_Init();
		uninitialized = 0;
		SysTick_Init(&Play_Song, Current_Tempo); // Initialize tempo counter.
	}
	Change_Tempo(current_tempo);
}

// ***************** Rewind ****************
// Restart the current song.
// Inputs:  none
// Outputs: none
void Rewind(){
	//Pause();
		current_Soprano_Beats = 0; current_Alto_Beats = 0; current_Third_Beats = 0; // Reset current note beats.
		Song* currentSong = &testSongs.songs[2];
		currentSong->soprano_Notes = currentSong->first_Soprano_Note; // Reset notes to first note.
		currentSong->alto_Notes = currentSong->first_Alto_Note;
		currentSong->third_Notes = currentSong->first_Third_Note;
		SysTick_Init(&Play_Song, Current_Tempo);
		Playing = 1;
}
