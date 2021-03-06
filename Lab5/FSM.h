// FSM.h
// Runs on LM4F120/TM4C123
// The state machine underlying the alarm clock project.
//	Based on switch input, allows a user to navigate between
//	several screens including a main screen with time display,
//	alarm/time set screens, and song choice screens.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016


extern char Toggle_Play;

// **************Next_State*********************
// Given the current state and the current input, find the next state of the FSM.
// Input: Current state and current switch input.
uint32_t Next_State(uint32_t current_state, uint32_t keyInputs);

// **************currentSongPos*********************
// The position of the song screen selection. Can be moved up and down via
//		switch input.
extern volatile int currentSongPos;

// **************lastSongPos*********************
// The position of the last played song. 
extern volatile int lastSongPos;

// **************currentMode*********************
// The current selected mode (sine, flute, trumpet).
extern volatile int currentMode;
