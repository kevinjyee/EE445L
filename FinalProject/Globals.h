// Globals.h
// Runs on LM4F120/TM4C123
// A file which contains all globals used by the Soundtrack
//		to My Life project.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

#include "stdint.h"

// **************MainMenuPos*********************
// The current position of the main menu cursor.
extern uint8_t MainMenuPos;

// **************currentSongPos*********************
// The position of the song screen selection. Can be moved up and down via
//		switch input.
extern volatile int currentSongPos;

// **************Current_Tempo*********************
// The tempo value of the current song.
extern volatile int16_t Current_Tempo;

// **************Playing*********************
// A boolean indicating the status of music (playing/paused).
extern volatile int Playing;

// **************Volume*********************
// A boolean indicating the status of music (playing/paused).
extern volatile int Volume;

// **************Step_Count*********************
// Current number of steps walked.
extern volatile uint32_t Step_Count;