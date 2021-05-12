// Lab10.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/16/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "SSD1306.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Music.h"
#include "SysTick.h"
#include "Timer0.h"
#include "Timer1.h"
#include "TExaS.h"
#include "Switch.h"
//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PB54                  (*((volatile uint32_t *)0x400050C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PB5       (*((volatile uint32_t *)0x40005080)) 
#define PB4       (*((volatile uint32_t *)0x40005040)) 
// TExaSdisplay logic analyzer shows 7 bits 0,PB5,PB4,PF3,PF2,PF1,0 
// edit this to output which pins you use for profiling
// you can output up to 7 pins
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PB54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// edit this to initialize which pins you use for profiling
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x22;      // activate port B,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF3,2,1
  GPIO_PORTB_DIR_R |=  0x30;   // output on PB4 PB5
  GPIO_PORTB_DEN_R |=  0x30;   // enable on PB4 PB5  
}
//********************************************************************************
uint32_t Position;
uint8_t DeadChecker = 0;
//uint8_t UpdateScore = 0;
uint8_t RocketToggler = 0;
uint8_t Language = 0; //Language = 0 -> English | Language = 1 -> Spanish
uint32_t ScoreTracker = 0;
uint32_t CoinTracker = 0;
uint8_t PlayBackground = 1;
uint8_t UpdateCoin;
uint8_t CoinArr[3] = {0x30, 0x30, 0x30};
extern uint32_t MailValue;
extern uint32_t MailStatus;
extern uint8_t RisingEdges;
extern uint8_t FallingEdges;
extern uint8_t TogglerSelection;
extern uint8_t PausePlay;
extern uint8_t NextButton;
extern uint32_t ScoreStorage;
void Draw(void);
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void (*MusicPointer)(void);
void Init(void);
void RandomSelection(void);

uint32_t Convert(uint32_t data){
  return (194*data)/4096+2;
}
uint8_t FlagEnglish = 0;
uint8_t FlagSpanish = 0;
void ChooseLanguage(void){
	while(NextButton == 0){
		while(MailStatus == 0){
		};
		MailStatus = 0;
		Position = Convert(MailValue);
		SSD1306_DrawBMP(0, 64, LanguageSelect, 0, SSD1306_WHITE);
		SSD1306_OutBuffer();
		if(Position >= 96){
			Language = 0;
			SSD1306_DrawBMP(35, 17, Arrow, 0, SSD1306_WHITE);
			FlagSpanish = 1;
			if(FlagEnglish == 1){
				Sound_Start(menumove, 663);
				FlagEnglish = 0;
			}
		}
		else{ //0-95
			Language = 1;
			SSD1306_DrawBMP(35, 25, Arrow, 0, SSD1306_WHITE);
			FlagEnglish = 1;
			if(FlagSpanish == 1){
				Sound_Start(menumove, 663);
				FlagSpanish = 0;
			}
		}
		SSD1306_OutBuffer();
	}
	Sound_Start(menuselect, 3971);
	SysTick_Wait10ms(200);
	if(Language == 0){
		SSD1306_DrawBMP(0, 64, MobiusStrip, 0, SSD1306_WHITE);
	}
	else{
		SSD1306_DrawBMP(0, 64, MobiusStripEspanol, 0, SSD1306_WHITE);
	}
	SSD1306_OutBuffer();
	while(NextButton == 1){
	};
}
	
void GamePause(void){
	while(PausePlay == 1){
		if(Language == 0){
			SSD1306_ClearBuffer();
			SSD1306_DrawBMP(0, 64, PauseScreen, 0, SSD1306_WHITE);
			SSD1306_OutBuffer();
			ScoreTracker = ScoreStorage;
		}
		else{
			SSD1306_ClearBuffer();
			SSD1306_DrawBMP(0, 64, PauseScreenEspanol, 0, SSD1306_WHITE);
			SSD1306_OutBuffer();
			ScoreTracker = ScoreStorage;
		}
	}
}

void GameOver(void){
	SSD1306_ClearBuffer();
	if(DeadChecker == 1){
		if(Language == 0){
			SSD1306_DrawBMP(0, 64, DeathScreen, 0, SSD1306_WHITE);
			SSD1306_OutBuffer();
			SSD1306_SetCursor(3, 2);
			SSD1306_OutString("Total Score:");
			uint32_t TotScore = ScoreTracker + 100*CoinTracker;
			SSD1306_SetCursor(15, 2);
			LCD_OutDecCoin(TotScore);
		}
		else{
			SSD1306_DrawBMP(0, 64, DeathScreen, 0, SSD1306_WHITE);
			SSD1306_OutBuffer();
			SSD1306_SetCursor(2, 2);
			SSD1306_OutString("Puntaje Total:");
			uint32_t TotScore = ScoreTracker + 10*CoinTracker;
			SSD1306_SetCursor(16, 2);
			LCD_OutDecCoin(TotScore);
		}
		MusicPointer = &Music_Start_Defeat;
		Timer0_Init(MusicPointer, 7272);
		}
		while(DeadChecker == 1){
			if(RisingEdges){
				SSD1306_ClearBuffer();
				DeadChecker = 0;
				ScoreTracker = 0;
				CoinTracker = 0;
				CoinArr[0] = 0x30;
				CoinArr[1] = 0x30;
				CoinArr[2] = 0x30;
				PlayBackground = 1;
				ChooseLanguage();
				Init();
				RandomSelection();
			}
		}	
}

typedef enum{dead, alive} status_t;
struct sprite{
	int32_t x;
	int32_t y;
	int32_t vx, vy;
	const uint8_t *image;
	status_t life;
};
typedef struct sprite sprite_t;
sprite_t Yerra;
sprite_t LaserObstacle;
sprite_t VertLaserObstacle;
sprite_t RocketObstacle;
sprite_t SmokeTrail;
sprite_t WarningSign;
sprite_t Coins;
sprite_t CoinSymbol;
int NeedToDraw;
uint8_t YerraW = 16;
uint8_t YerraH = 16;
uint8_t MoveToggler = 0;
uint8_t BlankToggler = 0;
uint8_t FlashCounter = 6;
void Init(void){
	Yerra.x = 20;
	Yerra.y = 62;
	Yerra.image = YIdle;
	Yerra.life = alive;
	
	LaserObstacle.x = 105;
	LaserObstacle.y = 62 - 5*(Random()/24); 
	LaserObstacle.image = LightningRod; 
	LaserObstacle.life = alive;
	
	VertLaserObstacle.x = 120;
	VertLaserObstacle.y = 62 - 4*(Random()/26); 
	VertLaserObstacle.image = VertLightningRod; 
	VertLaserObstacle.life = alive;
	
	WarningSign.x = 111;
	WarningSign.y = 62 - 10*(Random()/52);
	WarningSign.image = WarningSymbol;
	WarningSign.life = dead;
	
	RocketObstacle.x = 100;
	RocketObstacle.y = WarningSign.y;
	RocketObstacle.image = Rocket;
	RocketObstacle.life = alive;
	
	SmokeTrail.x = RocketObstacle.x + 24;
	SmokeTrail.y = RocketObstacle.y;
	SmokeTrail.image = RocketSmoke;
	SmokeTrail.life = dead;

	Coins.x = 110;
	Coins.y = 62 - 5*(Random()/24); 
	Coins.image = Coin;
	Coins.life = alive;
	
	CoinSymbol.x = 0;
	CoinSymbol.y = 5;
	CoinSymbol.image = SmallCoin;
	CoinSymbol.life = alive;
	
	Yerra.vx = 1;
	Yerra.vy = 2;
	LaserObstacle.vx = -1;
	LaserObstacle.vy = 0;
	RocketObstacle.vx = -2;
	RocketObstacle.vy = 0;
	SmokeTrail.vx = -2;
	VertLaserObstacle.vx = -1;
	VertLaserObstacle.vy = 0;
	Coins.vx = -1;
	Coins.vy = 0;
}

void RandomSelection(void){
	uint8_t GenRandom = Random()/86;
	if(GenRandom == 0){
		RocketObstacle.life = dead;
		LaserObstacle.life = alive;
		VertLaserObstacle.life = dead;
	}
	if(GenRandom == 1){
		LaserObstacle.life = dead;
		WarningSign.life = alive;
		VertLaserObstacle.life = dead;
	}
	if(GenRandom == 2){
		LaserObstacle.life = dead;
		RocketObstacle.life = dead;
		VertLaserObstacle.life = alive;
	}
}

void ReviveLightning(void){
	LaserObstacle.life = alive;
	LaserObstacle.x = 105;
	LaserObstacle.y = 62 - 5*(Random()/24);
}

void ReviveVertLightning(void){
	VertLaserObstacle.life = alive;
	VertLaserObstacle.x = 120;
	VertLaserObstacle.y = 62 - 4*(Random()/26);
}

void ReviveRocket(void){
	RocketObstacle.life = alive;
	RocketObstacle.x = 100;
	RocketObstacle.y = WarningSign.y;
}

void ReviveSmoke(void){
	SmokeTrail.life = alive;
	SmokeTrail.x = RocketObstacle.x + 24;
	SmokeTrail.y = RocketObstacle.y;
}

void ReviveWarning(void){
	WarningSign.life = alive;
	WarningSign.x = 111;
	WarningSign.y = 62 - 10*(Random()/52);
}

void ReviveCoins(void){
	Coins.life = alive;
	Coins.x = 110;
	Coins.y = 62 - 5*(Random()/24); 
}

void LaserCollision(void){
	uint8_t LightningW = 30;
	uint8_t LightningH = 6;
	//Checking collision along x
	if(((Yerra.x + YerraW) > (LaserObstacle.x+2)) && (Yerra.x < (LaserObstacle.x + LightningW))){
		//Check collision along y
		if(((Yerra.y - YerraH) < LaserObstacle.y) && (Yerra.y > (LaserObstacle.y - LightningH))){
			Yerra.life = dead;
			Sound_Start(zap, 8080);
		}
	}
}

void VertLaserCollision(void){
	uint8_t VertLightningW = 6;
	uint8_t VertLightningH = 25;
	//Checking collision along x
	if(((Yerra.x + YerraW) > (VertLaserObstacle.x+2)) && (Yerra.x < (VertLaserObstacle.x + VertLightningW))){
		//Check collision along y
		if(((Yerra.y - YerraH) < VertLaserObstacle.y) && (Yerra.y > (VertLaserObstacle.y - VertLightningH))){
			Yerra.life = dead;
			Sound_Start(zap, 8080);
		}
	}
}

void RocketCollision(void){
	uint8_t RocketW = 30;
	uint8_t RocketH = 11;
	//Checking collision along x
	if(((Yerra.x + YerraW) > (RocketObstacle.x+3)) && (Yerra.x < (RocketObstacle.x + RocketW))){
		//Check collision along y
		if(((Yerra.y - YerraH) < RocketObstacle.y) && (Yerra.y > (RocketObstacle.y - RocketH))){
			Yerra.life = dead;
			Sound_Start(explosion, 2000);
		}
	}
}

void CoinCollision(void){
	uint8_t CoinW = 10;
	uint8_t CoinH = 10;
	//Checking collision along x
	if(((Yerra.x + YerraW) > (Coins.x+2)) && (Yerra.x < (Coins.x + CoinW))){
		//Check collision along y
		if(((Yerra.y - YerraH) < Coins.y) && (Yerra.y > (Coins.y - CoinH))){
			if(Yerra.life == dead){
				return;
			}
			CoinTracker++;
			UpdateCoin = 1;
			Coins.life = dead;
			Sound_Start(coinsound, 3919);
		}
	}
}

void PlayWarning(void){
	if(WarningSign.life == alive){
		Sound_Start(dangeralarm, 721);
	}
}

void PlayRocket(void){
	if(RocketObstacle.life == alive){
		Sound_Start(rocket, 8172);
	}
}

uint8_t ScoreArr[6] = {0x30};
void DisplayScore(void){
	char *score;
	score = LCD_OutDec(ScoreTracker);
	ScoreArr[0] = *score;
	ScoreArr[1] = *(score + 1);
	ScoreArr[2] = *(score + 2);
	ScoreArr[3] = *(score + 3);
	ScoreArr[4] = *(score + 4);
	ScoreArr[5] = *(score + 5);
}


void DisplayCoin(void){
	if(UpdateCoin == 1){
		if((CoinTracker % 100) != 0){
			//CoinArr[0] = 0x30;
			if((CoinTracker % 10) != 0){
				//CoinArr[1] = 0x30;
				CoinArr[2]++;
			}
			else{
				CoinArr[1]++;
				CoinArr[2] = 0x30;
			}
		}
		else{
			CoinArr[0]++;
			CoinArr[1] = 0x30;
			CoinArr[2] = 0x30;
		}
	}
	UpdateCoin = 0;
}

void MoveX(void){
	if(Yerra.life == alive){
		MoveToggler ^= 1;
		NeedToDraw = 1;
		
		if(MoveToggler == 1){
			Yerra.image = YLeftS;
		}
		else {
			Yerra.image = YIdle;
		}
		
		if(Position < 25){
			Yerra.x += Yerra.vx;
			if(Yerra.x>110){
				Yerra.vx = 0;
			}
			else{
				Yerra.vx = 1;
			}
		}
		else if(Position < 170){
			Yerra.x += 0;
		}
		else{
			if(Yerra.x<1){
				Yerra.vx = 0;
			}
			else{
				Yerra.vx = 1;
			}
			Yerra.x -= Yerra.vx;
		}
		
		if(RisingEdges){
			if(Yerra.y > 16){
				Yerra.y -= Yerra.vy;
			}
			Yerra.image = YJetUp;
			Sound_Start(jetpack, 2092);
		}
		else if(FallingEdges){
			if(Yerra.y < 62){
				Yerra.y += Yerra.vy;
				Yerra.image = YIdle;
			}
		}
	}
	
	if(LaserObstacle.life == alive){
		NeedToDraw = 1;
		if((LaserObstacle.y>62) || (LaserObstacle.y<0) || (LaserObstacle.x<0) || (LaserObstacle.x>127)){
			LaserObstacle.life = dead;
			if(LaserObstacle.life == dead){
				ReviveLightning();
				RandomSelection();
			}
		}
		else{
			LaserObstacle.x += LaserObstacle.vx;
			LaserObstacle.y += LaserObstacle.vy;
		}
		LaserCollision();
	}
	
	if(VertLaserObstacle.life == alive){
		NeedToDraw = 1;
		if((VertLaserObstacle.y>62) || (VertLaserObstacle.y<0) || (VertLaserObstacle.x<0) || (VertLaserObstacle.x>127)){
			VertLaserObstacle.life = dead;
			if(VertLaserObstacle.life == dead){
				ReviveVertLightning();
				RandomSelection();
			}
		}
		else{
			VertLaserObstacle.x += VertLaserObstacle.vx;
			VertLaserObstacle.y += VertLaserObstacle.vy;
		}
		VertLaserCollision();
	}
	
	if(WarningSign.life == alive){
		NeedToDraw = 1;
		BlankToggler ^= 1;
		if(BlankToggler == 1){
			WarningSign.image = WarningSymbol;
		}
		else{
			WarningSign.image = FlashBlank;
		}
		FlashCounter--;
		if(FlashCounter == 0){
			WarningSign.life = dead;
			FlashCounter = 6;
			RocketObstacle.life = alive;
		}
	}
	
	if(RocketObstacle.life == alive){
		NeedToDraw = 1;
		RocketToggler ^= 1;
		if(RocketToggler == 0){
			RocketObstacle.image = Rocket;
		}
		else{
			RocketObstacle.image = RocketNoFin;
		}
		
		if(RocketObstacle.x <= 90){
			SmokeTrail.life = alive;
		}
		if((RocketObstacle.y>62) || (RocketObstacle.y<0) || (RocketObstacle.x<0) || (RocketObstacle.x>127)){
			RocketObstacle.life = dead;
			SmokeTrail.life = dead;
			if(RocketObstacle.life == dead){
				ReviveRocket();
				RandomSelection();
			}
			if(SmokeTrail.life == dead){
				ReviveSmoke();
				RandomSelection();
			}
		}
		else{
			RocketObstacle.x += RocketObstacle.vx;
			RocketObstacle.y += RocketObstacle.vy;
			SmokeTrail.x += SmokeTrail.vx;
		}
		RocketCollision();
	}
	
	if(Coins.life == dead){
		ReviveCoins();
	}
	if(Coins.life == alive){
		NeedToDraw = 1;
		if((Coins.y>62) || (Coins.y<0) || (Coins.x<0) || (Coins.x>118)){
			Coins.life = dead;
			if(Coins.life == dead){
				ReviveCoins();
			}
		}
		else{
			Coins.x += Coins.vx;
			Coins.y += Coins.vy;
		}
		CoinCollision();
	}
	if(ScoreTracker == 500){
		LaserObstacle.vx = -2;
		RocketObstacle.vx = -3;
		SmokeTrail.vx = -3;
		VertLaserObstacle.vx = -2;
		Coins.vx = -2;
	}
	if(ScoreTracker == 1000){
		LaserObstacle.vx = -3;
		RocketObstacle.vx = -4;
		SmokeTrail.vx = -4;
		VertLaserObstacle.vx = -3;
		Coins.vx = -3;
	}
	if(ScoreTracker == 1500){
		LaserObstacle.vx = -4;
		RocketObstacle.vx = -5;
		SmokeTrail.vx = -5;
		VertLaserObstacle.vx = -4;
		Coins.vx = -4;
	}
}	

void Draw(void){
	SSD1306_ClearBuffer();
	SSD1306_DrawBMP(0, 64, FullBackgroundNoPillar, 0, SSD1306_WHITE);
	if(Yerra.life == alive){
		SSD1306_DrawBMP(Yerra.x, Yerra.y, Yerra.image, 0, SSD1306_WHITE);
	}
	if(LaserObstacle.life == alive){
		SSD1306_DrawBMP(LaserObstacle.x, LaserObstacle.y, LaserObstacle.image, 0, SSD1306_WHITE);
	}
	if(VertLaserObstacle.life == alive){
		SSD1306_DrawBMP(VertLaserObstacle.x, VertLaserObstacle.y, VertLaserObstacle.image, 0, SSD1306_WHITE);
	}
	if(WarningSign.life == alive){
		SSD1306_DrawBMP(WarningSign.x, WarningSign.y, WarningSign.image, 0, SSD1306_WHITE);
	}
	if(RocketObstacle.life == alive){
		SSD1306_DrawBMP(RocketObstacle.x, RocketObstacle.y, RocketObstacle.image, 0, SSD1306_WHITE);
	}
	if(SmokeTrail.life == alive){
		SSD1306_DrawBMP(SmokeTrail.x, SmokeTrail.y, SmokeTrail.image, 0, SSD1306_WHITE);
	}
	if(Coins.life == alive){
		SSD1306_DrawBMP(Coins.x, Coins.y, Coins.image, 0, SSD1306_WHITE);
	}
	if(CoinSymbol.life == alive){
		SSD1306_DrawBMP(CoinSymbol.x, CoinSymbol.y, CoinSymbol.image, 0, SSD1306_WHITE);
	}
	if(Yerra.life == dead){
		DeadChecker = 1;
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(Yerra.x, Yerra.y, YDeath1, 0, SSD1306_WHITE);
		SysTick_Wait10ms(10); 
		SSD1306_OutBuffer();
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(Yerra.x, Yerra.y, YDeath2, 0, SSD1306_WHITE);
		SysTick_Wait10ms(10); 
		SSD1306_OutBuffer();
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(Yerra.x, Yerra.y, YDeath3, 0, SSD1306_WHITE);
		SysTick_Wait10ms(10); 
		SSD1306_OutBuffer();
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(Yerra.x, Yerra.y, YDeath4, 0, SSD1306_WHITE);
		SysTick_Wait10ms(10); 
		SSD1306_OutBuffer();
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(Yerra.x, Yerra.y, YDeath5, 0, SSD1306_WHITE);
		SysTick_Wait10ms(10); 
		SSD1306_OutBuffer();
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(Yerra.x, Yerra.y, YDeath6, 0, SSD1306_WHITE);
		SysTick_Wait10ms(10); 
		SSD1306_OutBuffer();
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(Yerra.x, Yerra.y, YDeath7, 0, SSD1306_WHITE);
		SysTick_Wait10ms(10); 
		SSD1306_OutBuffer();
		LaserObstacle.life = dead;
		VertLaserObstacle.life = dead;
		RocketObstacle.life = dead;
		SSD1306_ClearBuffer();
		SSD1306_OutClear();
	}
	PlayWarning();
	PlayRocket();
	DisplayCoin();
	for(int8_t i=0; i<3; i++){
		SSD1306_DrawChar(10 + (6*1*i), 0, CoinArr[i], SSD1306_WHITE);
	}
	
	DisplayScore();
	for(uint8_t i=0; i<6 && ScoreArr[i] <= 0x39 && ScoreArr[i] >= 0x30; i++){
		SSD1306_DrawChar(105 + (6*i), 0, ScoreArr[i], SSD1306_WHITE);
	}
	SSD1306_OutBuffer();
	NeedToDraw = 0;
}

void PeriodicTask(void){
	if(Yerra.life == alive){

		ScoreTracker++;
	}
}

int main(void){
	//uint32_t time=0;

  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();	
  //Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1
	ADC_Init(SAC_32); //Turn on ADC, set channel to 5
	Random_Init(ADC_In());
	Timer1_Init(&PeriodicTask, 7999999);	
	EdgeCounter_Init();
	SysTick_Init();
  SSD1306_ClearBuffer();
  EnableInterrupts();
	MusicPointer = &Music_Stop;
	Timer0_Init(MusicPointer, 7272);
	ChooseLanguage();
	Init();
	RandomSelection();
  while(1){
		if(PlayBackground == 1){
			MusicPointer = &Music_Start_Background_Main;
			Timer0_Init(MusicPointer, 7272);
			PlayBackground = 0;
		}
		
		while(MailStatus == 0){
		};
		MailStatus = 0;
		Position = Convert(MailValue);
		Draw();
		if((GPIO_PORTE_DATA_R&0x01) == 0x01){
			MoveX();
			Draw();
		}
		else{
			MoveX();
		}
		GamePause();
		GameOver();
  }
}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
