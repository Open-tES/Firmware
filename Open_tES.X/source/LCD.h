/****************************************************************/
/************************  LCD DISPLAY   ************************/
/****************************************************************/

#include <pic.h>
#include <string.h>
#include <htc.h>	// Required to interface with delay routines
#include <xc.h>
#include "misc.h"

//#define _XTAL_FREQ 32000000
#define _XTAL_FREQ 32000000
#include "misc.h"
#include "special_chars.h"

#ifndef LCD_H
#define LCD_H

#define  LCD_DB PORTD
#define  LCD_TRISDB TRISD

#define LCD_E RC0 
#define LCD_RW RC1
#define LCD_RS RC2

void LatchData(char Data);

void DisplayClear(void);

void ReturnHome(void);

void EntryModeSet(char ID_SH);

void DisplayOnOff(char D_C_B);
//D=H display on C=H Cursor on B=H Display blinking
//D_C_B ->0b0000 0DCB

void Shift(char SC_RL);

void SetFunction(char DL_N_F);

void SetCGRamAddress(char Address);

void SetDDRamAddress(char Address);

char ReadBusyFlag(void);
// 1 LCD if busy else 0
void CreatChar(char Address,const unsigned char *Map);

void WriteData(char Data);

char ReadData(void);

void WriteDataOnAddress(char Data,char Address);
void WriteStringOnAddress(char String[],char Address);
void MoveCursorOnLine(char Line);

void WriteStringOnLine(char BufferLcd[],char String[],char align,char shift);
void WriteNumberOnLine(char BufferLcd[],char Buffer[],unsigned int Number,char align,char shift,char Unit[]);
void WriteFloatOnLine(char BufferLcd[],char Buffer[],unsigned int Number,char PointPosition,char align,char shift,char Unit[]);
//Line =0-3 else 0
//Option : l=left, c=center,r=right and else left

void InitLCD (void);



#endif