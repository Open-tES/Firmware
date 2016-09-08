/* 
 * File:   misc.c
 * Author: pandrieu
 *
 * Created on February 17, 2015, 4:07 PM
 */

#include "misc.h"

void config_pic(void){
    /**************************OSCILLATOR****************************/
    OSCCON=OSCCON & 0x87;
    OSCCON=OSCCON | 0x70;
    /**************************MISC****************************/
    RCIF=0;
    INTF=0;
    TXREG=0;
    OPTION_REG=0x40; // Pull-ups individuals, prescaler on Timer0  128
    TMR0=0x00;
    /**************configuration of A2D converter********************/
    /*AN0->current  AN1-> Voltage*/
    ADRESH=0x00;
    ADRESL=0x00;
    PIR1=0x00;
    ADCON0=0x01; // CAN on , reading on RA0 (AN0)
    ADCON1=0xA0; // Right justified, A/D conversion clock Fosc/32
    ANSELA=0x3F; // Pin of PORT A analog input
    WPUB=0x0F;  //  weak pull-up on RB0/INT
    CCPTMRS1=0x00;
    TRISB=0xFF;
    TRISC=0x80;
    PIE2=0x00;
    TRISD=0x00;
    BAUDCON=0x00;
    CCPR5L=0x00;
    /**************configuration of USART mode Async*****************/
    TRISC=TRISC & 0xBF; //RC6/TX output
    TRISC=TRISC | 0x80; //RC7/RX input
    
    //Register TXSTA
    TX9=0;      //0->8bits 1->9bits
    TXEN=0;     //Trasnmition disable
    SYNC=0;     //1 = synchrone, 0 = asynchrone
    BRGH=1;     //HighSpeed
    BRG16=1;   //SPBRG on 16 Bits
    SPBRG=0x40; // bauds speed 9600
    SPBRGH=0x03; // bauds speed 9600
    TXIE=1;
    //Register RCSTA
    SPEN=1;      //Serial port enable
    RX9=0; // 0->8bits 1->9bits
    CREN=0;     //Reception disable
    ADDEN=0;    //Disable adress detection
    RCIE=0;
    
    TRISA=0xFF;
    TRISB=0xFF;
    PORTB=0x00;
    ANSELB=0x00;
    ANSELD=0x00;
    TRISD=0x00;
    TRISE=0b000; 
    TRISC=0x80;
    CCP5CON=0x0C; //PWM on CCP5

    T2CON=0x054; // TMR2 On, prescaler 1
    PR2=0xFF;
    EECON1=0x00;
    PORTD=0x0F;
    INTCON=0xD0;    	  // global Interruptions and Peripheral active
    PIE1=0x20;  			// interrupt enable
    TRISB=TRISB&0xDF;
    /*TIMER 4 for fade in and fade OUT :1ms*/
    T4CON=0x07;  // Prescaler 64,postscaler 1 ->1ms for 32MHz
    PR4=125;
    TMR4IF=0;
    TMR4IE=1;
    TMR4ON=1;
    /*TIMER 6 for refreshing display : 1/100s*/
    T6CON=0x4B;  // Prescal 10,postscale 64
    PR6=125;//no drift
    TMR6IF=0;
    TMR6IE=1;
    /*TIMER 1 for Stimulation duration : 1/16s */
    T1CON=0x34;// clock (Fosc/4), prescale1:8, timer stop
    TMR1L=0xDB;
    TMR1H=0x0B;
    TMR1IF=0;
    TMR1IE=1;

    /*Interruptions enable*/
    PIE1=PIE1|0x40;
    GIE=1;
    PEIE=1;
}

void set_pwm(unsigned int intensity){
unsigned int tmp;
    if (intensity==0){
        TRISE=3;
        CCPR5L=0x00;
        CCP5CON=CCP5CON&0xCF;
    }
else{
    TRISE=0x00;
    CCPR5L=intensity>>2;
    tmp=intensity&0x0003;
    CCP5CON=0x0C|tmp<<4;
    }
}

void Num2Str(char Buffer[],unsigned int Number,char PointPosition,char Unit[]){
    char StrSize=0;
    UInt2Str((unsigned int)Number,Buffer);
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
}

void Padding(char Buffer[],char BufferSize,char PadCharacter){
    while (BufferSize>0){
        Buffer[--BufferSize]=PadCharacter;
    }
}

void UInt2Str(unsigned int Value,char Buffer8[]){
    unsigned char i=7;
    unsigned char j=0;
    do{
        Buffer8[i]=Value%10;
        Value=Value/10;
        i--;
    }
        while(Value>0);
    while(i<7){
    i++;
    Buffer8[j]=Buffer8[i]+48;
    j++;
    }
    Buffer8[j]=0;
}

void UChar2StrWithSpacePadding(unsigned char Value,char Buffer4[]){
    Buffer4[4]=0;
    char i=3;
    do{
        Buffer4[i]=Value%10+48;
        Value=Value/10;
        i--;
    }
    while(Value>0);
    while(i<=0){
    Buffer4[i]=' ';
    i--;
    }
}