typedef struct
{
	int id;								// For use in menu selection
	char name[20];					// For use in printing name to LCD.
	const unsigned short image[]; //For printing the image in the screen
} Song;  /* Song structs */