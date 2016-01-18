#ifndef CAR_SPEC_H
#define CAR_SPEC_H 


const unsigned char CharArrowUp[8]=
{
0b00000100,
0b00001110,
0b00010101,
0b00000100,
0b00000100,
0b00000100,
0b00000000,
0b00000000
};

const unsigned char CharArrowDown[8]=
{
0b00000000,
0b00000000,
0b00000100,
0b00000100,
0b00000100,
0b00010101,
0b00001110,
0b00000100
};


const unsigned char CharMicro[8]=   //µ
{
0b00000000,
0b00000000,
0b00010001,
0b00010001,
0b00010001,
0b00010011,
0b00011101,
0b00010000
};


const unsigned char CharPause[8]=	// Pause
{
0b00000000,
0b00011011,
0b00011011,
0b00011011,
0b00011011,
0b00011011,
0b00011011,
0b00000000
};

const unsigned char CharStop[8]=
{
0b00000000,				// Stop
0b00000000,
0b00011111,
0b00011111,
0b00011111,
0b00011111,
0b00011111,
0b00000000
};

const unsigned char CharOmega[8]=
{
0b00000000,				// Omega
0b00000000,
0b00001110,
0b00010001,
0b00010001,
0b00001010,
0b00011011,
0b00000000
};

const unsigned char CharPlay[8]=			// Play
{
0b00010000,			
0b00011000,
0b00011100,
0b00011110,
0b00011100,
0b00011000,
0b00010000,
0b00000000
};



#endif