

typedef struct
{
	int ID;
	char* SongName;
	uint16_t* Graphic;
} SongChoice;



uint32_t SongScreen(uint32_t input, struct SongChoice data);