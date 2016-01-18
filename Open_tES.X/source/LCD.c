#include "LCD.h"


void LatchData(char Data)
	{
        LCD_TRISDB=0x00;
        LCD_DB=Data;
        NOP();
	LCD_E=1;
        NOP();NOP();NOP();// 250ns
	LCD_E=0;
        NOP();NOP();NOP();//250ns
	}
void DisplayClear(void)
	{
    	while(ReadBusyFlag()==1)
		{}
    	LCD_RS=0;
        NOP();
        LCD_RW=0;
	LatchData(0x01);
	}
void ReturnHome(void)
{
	LCD_TRISDB=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=0;
	LCD_RW=0;
	LatchData(0x02);
}

void EntryModeSet(char ID_SH)
{
	LCD_TRISDB=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=0;
	LCD_RW=0;
	LatchData(0x04|ID_SH);
}

void DisplayOnOff(char D_C_B)
{
	LCD_TRISDB=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=0;
	LCD_RW=0;
	LatchData(0x08|D_C_B);
}

void Shift(char SC_RL)
	{
	LCD_TRISDB=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=0;
	LCD_RW=0;
	LatchData(0x10|(SC_RL<<2));
	}

void SetFunction(char DL_N_F)
	{
	LCD_TRISDB=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=0;
	LCD_RW=0;
	LatchData(0x20|(DL_N_F<<2));
	}

void SetCGRamAddress(char Address)
	{
	LCD_TRISDB=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=0;
	LCD_RW=0;
	LatchData(0x40|Address);
	}


char ReadBusyFlag(void)
	{
	char tmp=0;
	LCD_TRISDB=0xff;
	LCD_RS=0;
        NOP();
	LCD_RW=1;
        NOP();
	LCD_E=1;
        NOP();NOP();//250ns
	tmp=LCD_DB & 0x80;
	LCD_E=0;
	LCD_TRISDB=0x00;
        if (tmp==0x80){
            NOP();NOP();
            return 1;
            }
        else{
            NOP();NOP();
            return 0;
            }
	}



char ReadData(void)
	{
	char tmp=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=1;
	LCD_RW=1;
        NOP();
	LCD_TRISDB=0xff;
	LCD_E=1;
        NOP();
	tmp=LCD_DB;
	LCD_E=0;
	LCD_TRISDB=0x00;
	return tmp;
	}

void SetDDRamAddress(char Address)
	{
	LCD_TRISDB=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=0;
        NOP();
	LCD_RW=0;
	LatchData(0x80|Address);
	}

void WriteData(char Data)
	{
	LCD_TRISDB=0x00;
	while(ReadBusyFlag()==1)
		{}
	LCD_RS=1;
        NOP();
	LCD_RW=0;
	LatchData(Data);
	}

void WriteDataOnAddress(char Data,char Address)
	{
	SetDDRamAddress(Address);
	WriteData(Data);
	}

void WriteStringOnAddress(char String[],char Address)
	{
	unsigned char StrSize=0;
        unsigned char i=0;
        StrSize=strlen(String);
	SetDDRamAddress(Address);
	while(i<StrSize)
		{
		WriteData(String[i]);
                i++;
		}
	}
void WriteStringOnLine(char BufferLcd[],char String[],char align,char shift)
	{
	unsigned char Address=0;
        signed char n=strlen(String);
	switch(align)//Align
		{
		case 'l' : break;//left
		case 'c' : Address=(Address+8)-(n>>1);break;//Center
		case 'r' : Address=Address+16-n;break;//right
		default : break;
		}
        Address=Address+shift;//Shift
	while(--n>=0)
		{
		BufferLcd[Address+n]=String[n];//%optimized
		}
	}
void WriteNumberOnLine(char BufferLcd[],char Buffer[],unsigned int Number,char align,char shift,char Unit[]){
    itoa(Buffer,(unsigned int)Number,10);
    strcat(Buffer,Unit);
    WriteStringOnLine(BufferLcd,(char*)"- ",align,0);
    WriteStringOnLine(BufferLcd,Buffer,align,shift);
}


void WriteFloatOnLine(char BufferLcd[],char Buffer[],unsigned int Number,char PointPosition,char align,char shift,char Unit[]){
    char StrSize=0;
    UInt2Str(Number,Buffer);
    if (PointPosition>0){
        StrSize=strlen(Buffer);
        StrSize++;
        PointPosition++;
        while(PointPosition--){
            Buffer[StrSize]=Buffer[StrSize-1];
            StrSize--;
        }
        Buffer[StrSize]='.';
    }
    strcat(Buffer,Unit);
    WriteStringOnLine(BufferLcd,Buffer,align,shift);
}


void MoveCursorOnLine(char Line)
{
	switch (Line)
		{
		case 0 : SetDDRamAddress(0x00); break;
		case 1 : SetDDRamAddress(0x40); break;
		case 2 : SetDDRamAddress(0x14);break;
		case 3 : SetDDRamAddress(0x54);break;
		default : SetDDRamAddress(0x00);break;
		}
		
}

void CreatChar(char Address,const unsigned char *Map)
{
    unsigned char i;
    if(Address<=8)
	{
	SetCGRamAddress(Address*8);
	for(i=0;i<8;i++)
		{
                __delay_ms(1);
		WriteData(Map[i]);
                __delay_ms(1);
		}
	}
}


void InitLCD (void)
	{
        __delay_ms(15);
        SetFunction(0x07);
        __delay_ms(5);
        SetFunction(0x07);
        __delay_us(200);
	SetFunction(0x07);
	DisplayOnOff(0x00);
        DisplayClear();
        EntryModeSet(0x02);
        __delay_ms(2);
	DisplayOnOff(0x04);
	__delay_us(50);
        CreatChar(1,CharArrowUp);
        CreatChar(2,CharArrowDown);
        CreatChar(3,CharMicro);
        CreatChar(4,CharPause);
        CreatChar(5,CharStop);
        CreatChar(6,CharOmega);
        CreatChar(7,CharPlay);
	}


