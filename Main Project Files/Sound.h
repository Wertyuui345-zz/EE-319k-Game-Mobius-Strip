// Sound.h
// Runs on TM4C123 
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// 1/14/21
#ifndef _Sound_h
#define _Sound_h
#include <stdint.h>

#define explodeln 2092
#define jetpackln 2092
#define footstep1ln 536
#define	footstep2ln 548
#define coinln 3919
#define menuselectln 3971
#define menumoveln 3971
#define dangeralarmln 721
#define zapln 8080
#define rocketln 8172

// Header files contain the prototypes for public functions 
// this file explains what the module does
void Sound_Init(void);
//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the timer interrupt.
// It starts the sound, and the timer ISR does the output
// feel free to change the parameters
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count);


extern uint32_t startFlag;
extern uint32_t soundFlag;
extern const uint8_t *TrackPointer;
extern uint32_t EffectCount;

extern const uint8_t explosion[2092];
extern const uint8_t jetpack[2092];
extern const uint8_t footstep1[536];
extern const uint8_t footstep2[548];
extern const uint8_t coinsound[3919];
extern const uint8_t menumove[663];
extern const uint8_t menuselect[3971];
extern const uint8_t dangeralarm[721];
extern const uint8_t rocket[8172];
extern const uint8_t zap[8080];

#endif


