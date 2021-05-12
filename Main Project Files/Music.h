/*
Header file for Music Init

*/
#include <stdint.h>

extern uint8_t MusicFlag;
extern uint32_t count;

void Music_Start_Background_Main(void);
void Music_Start_Defeat(void);
void Music_Init(void);
void Music_Stop(void);

