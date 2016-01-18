/* 
 * File:   diverse.h
 * Author: pandrieu
 *
 * Created on February 17, 2015, 4:07 PM
 */

#ifndef MISC_H
#define	MISC_H

#include<pic.h>
#include<htc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
//Configuration I/O port timer RS232 PWM
void config_pic(void);

void set_pwm(unsigned int intensity);

void Num2Str(char Buffer[],unsigned int Number,char PointPosition,char Unit[]);
//This funtion converts Number into string strored into Buffer
//A point can be add by specifing its position "PointPosition" 0-> no point
//String "Unit" is concatenated at the end
//This function automatically add the char 0x13 "\n" at the end of string

void Padding(char Buffer[],char BufferSize,char PadCharacter);
void UInt2Str(unsigned int Value,char Buffer[]);
void UChar2StrWithSpacePadding(unsigned char Value,char Buffer4[]);
#endif	/* MISC_H */


