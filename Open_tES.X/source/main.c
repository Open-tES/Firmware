/* 
 * File:   main.c
 * Author: pandrieu
 *
 * Created on February 17, 2015, 3:37 PM
 */
#include "LCD.h"
#include "misc.h"
#include <stdio.h>
#include <stdlib.h>
#include <htc.h>	// Required to interface with delay routines
#include <xc.h>
#include <pic.h>
#include <string.h>
#define BUZZER RC4  // E=bit0 du PORTC
#define _XTAL_FREQ 32000000
// CONFIG1
#pragma config FOSC = HS       // Oscillator Selection (ECH, External Clock, High Power Mode (4-32 MHz): device clock supplied to CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable (All VCAP pin functionality is disabled)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)


//variable declaration
unsigned char FlagMenu=0x10;
unsigned char FlagRefreshMenu=1;
unsigned char FlagRefreshParameters=1;
unsigned char SnapShootButton=0;
unsigned int  StimDuration=1200;
unsigned int  VarStimDuration=500;
unsigned char FadeInDuration=2;
unsigned int FadeInSampleRate=20;
unsigned char FadeOutDuration=2;
unsigned int FadeOutSampleRate=20;
unsigned int TMR4SampleRate=20;
unsigned char Intensity=200;
unsigned int VarIntensity=0;
unsigned int TmpVarIntensity=0;//Used in Computer control mod to check value
unsigned char VarVoltage=125;
unsigned char VarImpedance=123;
unsigned char StimulationState=0;//0xAB A=1->request for B B= 0-stop 1-fadein 2-stim 3-fadeout 4-computer-controlled
         char StrBuffer_8[8];
         char StrBuffer_4[4];
         char SymbolPauseStop=0;//used for switch beetwen menu 0x12 and 0x13
unsigned char PostSacleTMR1=16;
unsigned char PostSacleTMR6=25;
unsigned char FlagCaptState=0;// 1 in progress //0 done
unsigned int CaptIntensity=0;
unsigned int CaptTension=0;
unsigned int CaptRAWIntensity=0;
unsigned int CaptRAWTension=0;
unsigned int CaptImpedance=0;
unsigned int ThresholdImpedance=100;//1=0.1Kohm
unsigned char TmpChar=0;
unsigned char i=0;
unsigned char FlagRefreshDisplay=0;
unsigned char DisplayCursor=0;
/*******************USART*************************/
char TX_Buffer[10];
char TX_Prefix=0;
signed char CounterForTxBuffer=-1; //-1 nothing to send  -2 TX_buffer is read else is used like counter to write buffer in TXREG    
char RC_Buffer[10];
char CHAR_RCREG=0;
signed char CounterForRCBuffer=0;
const char Terminator= '!';
unsigned char FlagRS232Enable=0;
unsigned char FlagCapt2TX_GO_DONE=0;

/*******************Limits of parameters***********/
const unsigned char LimSupFadeInOut= 60;
const unsigned char LimInfFadeInOut= 2;
const unsigned int LimSupStimDuration= 3600;
const unsigned int LimInfStimDuration= 10;
const unsigned int LimSupThresholdImpedance= 1500;//150Kohm
const unsigned int LimInfThresholdImpedance= 100;//10Kohm
const unsigned int LimInfIntensity= 10;
const unsigned int LimSupIntensity= 250;

/*******************Defaults parameters***********/
const unsigned char DefaultFadeInOutDuration=10;
const unsigned char DefaultIntensity=200;
const unsigned int DefaultStimDuration= 1200;
const unsigned int DefaultThresholdImpedance= 800;

/************************  LCD  ***************/
char BufferLCD_1[16]="";
char BufferLCD_2[16]="";
/*******************Adress for EEprom***********/
const char ADDR_StimDuration_L= 0x00;
const char ADDR_StimDuration_H= 0x01;
const char ADDR_Intensity= 0x03;
const char ADDR_FadeInDuration= 0x04;
const char ADDR_FadeOutDuration= 0x05;
const char ADDR_ThresholdImpedance_L= 0x06;
const char ADDR_ThresholdImpedance_H= 0x07;
const char ADDR_FlagRS232Enable= 0x08;

/***********************For Debbug **********************/
unsigned int a=0;

//Interrupt fonction
void interrupt INIT_erruptgetIT(void){
    if (INTF==1 && INTE==1){//Push-Button
        if (SnapShootButton < 0x0F){//check if previous process is done
            SnapShootButton=PORTB&0x0F;
            SnapShootButton=SnapShootButton|0x10;//Request for action
        }
        INTF=0;
    }//END Push-Button
    /****************************TIMER4**************************/
    if(TMR4IF==1 && TMR4IE==1 ){//1ms
        if(--TMR4SampleRate==0){
            switch(StimulationState){
                case 0x01://Fade In
                   if ((VarIntensity>>2)<Intensity){ //Intensity not reached
                       TMR4SampleRate=FadeInSampleRate;
                       set_pwm(++VarIntensity);
                   }
                   else{//Intensity  reached
                    StimulationState=0x12;
                   }
                   break;
                case 3://Fade OUT
                   if ((VarIntensity)>0){
                       TMR4SampleRate=FadeOutSampleRate;
                       set_pwm(--VarIntensity);
                   }
                   else{
                    StimulationState=0x10;
                   }
                   break;
            }
        }
        TMR4IF=0;
    }//end TMR4
    /****************************TIMER1**************************/
    if(TMR1IF==1 && TMR1IE==1){//(1/16) s
        TMR1L=0xDB;
        TMR1H=0x0B;
        if (--PostSacleTMR1==0){
            PostSacleTMR1=16;
            switch (StimulationState){
                case 0x02://Stimulation
                    if(--VarStimDuration==0){
                    StimulationState=0x10;
                    }
                    break;
                case 0x04://Computer control
                    VarStimDuration++;
                    break;
            }

         }
         TMR1IF=0;
    }//end TMR1
    /****************************TIMER6**************************/
    if(TMR6IF==1 && TMR6IE==1){//10ms
        TMR6IF=0;
        switch (--PostSacleTMR6){
            case 0 :
                FlagRefreshMenu=1;
                PostSacleTMR6=50;
                break;
            case 24 :
                if (FlagCaptState==0){
                FlagCaptState=1;
                GO_nDONE=1;//Start conversion AD
                }
                break;
            case 20 : BUZZER=0;break;
        }
    }//End TIMER6
    /****************************Capture**************************/
    if(ADIF && ADIE){//AN0 et AN1 ont été inversé dans eagle. La modif est faite dans la section suivante. vérifier qu'il n'y a pas d'autre bug
        if ((ADCON0&0x7C)==0x00){//Channel AN0 selected (voltage)
            ADCON0=ADCON0|0x04; // Channel AN1 selected (current)
            CaptRAWIntensity=(ADRESH<<8)+ADRESL;
            NOP();NOP();NOP();NOP();
            ADIF=0;
            GO_nDONE=1;   // Start next convertion on AN1
             }
        else if((ADCON0&0x7C)==0x04){//Channel AN1 selected (voltage)
            CaptRAWTension=(ADRESH<<8)+ADRESL;
            ADCON0=ADCON0&0x83;//Channel AN0 selected (current)
            ADIF=0;
            FlagCaptState=2;
        }
    }//End Capt
    /***************************RS232*****************************/
    if(TXIF==1 && TXIE==1){//Transmition
         //  <editor-fold defaultstate="collapsed" desc="RS232 Transmition">
        switch (CounterForTxBuffer){
            case -1 ://Nothing to send
                break;
            case -2 : //Send a prefix
                TXREG=TX_Prefix;
                CounterForTxBuffer=0;
                //BUZZER=1;
                break;
            default :
                if (TX_Buffer[CounterForTxBuffer]!=0x00){//Channel AN0 selected (current)
                 TXREG=TX_Buffer[CounterForTxBuffer++]; 
                }
                else{
                    TXREG='!';
                    CounterForTxBuffer=-1;//End of transmition
                    TXIE=0;
                }
                break;
        }
    }//End TX
    // </editor-fold>
    if(RCIF==1 && RCIE){//Reception
        //  <editor-fold defaultstate="collapsed" desc="RS232 reception">
        CHAR_RCREG=RCREG;
         if (CHAR_RCREG!='!' && CounterForRCBuffer<10){
             RC_Buffer[CounterForRCBuffer++]=CHAR_RCREG;
         }
         else{
             if (CounterForRCBuffer>=10){
                 CounterForRCBuffer=0;
                 CounterForTxBuffer=-2;
                 strcpy(TX_Buffer,(char*)"OFBR");
                 TX_Prefix='E';
             }
             else{
                 RC_Buffer[CounterForRCBuffer]=0;
                 switch(RC_Buffer[0]){
                     case 'I' :
                         RC_Buffer[0]='+';
                         TmpVarIntensity=atoi(RC_Buffer);
                         if (TmpVarIntensity<=1020){
                            VarIntensity=TmpVarIntensity;
                            set_pwm(VarIntensity);
                            TX_Prefix='I';
                            TX_Buffer[0]=0;
                         }
                         else{
                             TX_Prefix='E';
                             strcpy(TX_Buffer,(char*)"OOL");
                         }
                         CounterForTxBuffer=-2;//Transmission with prefix
                         break;
                     case '+' :
                         if (VarIntensity<1020){
                             set_pwm(++VarIntensity);
                             TX_Prefix='+';
                             TX_Buffer[0]=0;
                         }
                         else{
                             TX_Prefix='E';
                             strcpy(TX_Buffer,(char*)"OOL");
                         }
                         CounterForTxBuffer=-2;//Transmission with prefix
                         break;
                     case '-' :
                         if (VarIntensity>0){
                             TX_Prefix='-';
                             set_pwm(--VarIntensity);
                             TX_Buffer[0]=0;
                         }
                         else{
                             TX_Prefix='E';
                             strcpy(TX_Buffer,(char*)"OOL");
                         }
                         CounterForTxBuffer=-2;//Transmission with prefix
                         break;
                     case 'A' :
                             itoa(TX_Buffer,VarIntensity,10);
                             TX_Prefix='A';
                             CounterForTxBuffer=-2;//Transmission with prefix
                         break;
                     case 'v' :
                     case 'i' :
                             TX_Prefix=RC_Buffer[0];
                             FlagCapt2TX_GO_DONE=1;
                             if (FlagCaptState==0){
                                FlagCaptState=1;
                                GO_nDONE=1;//Start conversion AD
                             }
                             TX_Buffer[0]=0;
                         break;
                     default :
                        TX_Prefix='E';
                        strcpy(TX_Buffer,(char*)"NOTCMD");
                        CounterForTxBuffer=-2;//Transmission with prefix
                        break;
                 }//End switch
             }
             CounterForRCBuffer=0;
         }
     }//end reception
    // </editor-fold>
}//End IT


//Main function and infinity loop
void main(void) {
    config_pic();			// Configuration des registres du PIC
    InitLCD();
    BUZZER=0;
    // <editor-fold defaultstate="collapsed" desc="EEROM READING and parameter check">
    /**********************Load PARAMETERS FROM EEPROM*************************/
    StimDuration=(unsigned int)EEPROM_READ(ADDR_StimDuration_L);
    TmpChar=EEPROM_READ(ADDR_StimDuration_H);
    StimDuration+=((unsigned int)TmpChar)<<8;
    ThresholdImpedance=(unsigned int)EEPROM_READ(ADDR_ThresholdImpedance_L);
    TmpChar=EEPROM_READ(ADDR_ThresholdImpedance_H);
    ThresholdImpedance+=((unsigned int)TmpChar)<<8;
    FadeInDuration=EEPROM_READ(ADDR_FadeInDuration);
    FadeOutDuration=EEPROM_READ(ADDR_FadeOutDuration);
    Intensity=EEPROM_READ(ADDR_Intensity);
    FlagRS232Enable=EEPROM_READ(ADDR_FlagRS232Enable);
     /**********************Verification parameter values***********************/
    if(StimDuration<LimInfStimDuration ||StimDuration>LimSupStimDuration){
        StimDuration=DefaultStimDuration;
    }
    if(ThresholdImpedance<LimInfThresholdImpedance ||ThresholdImpedance>LimSupThresholdImpedance){
        ThresholdImpedance=DefaultThresholdImpedance;
    }
    if(FadeInDuration<LimInfFadeInOut ||FadeInDuration>LimSupFadeInOut){
        FadeInDuration=DefaultFadeInOutDuration;
    }
    if(FadeOutDuration<LimInfFadeInOut ||FadeOutDuration>LimSupFadeInOut){
        FadeOutDuration=DefaultFadeInOutDuration;
    }
    if (FlagRS232Enable>1){
        FlagRS232Enable=0;
    }
    if(Intensity<LimInfIntensity ||Intensity>LimSupIntensity){
        Intensity=DefaultIntensity;
    }
    /**********************Refresh parameter************************************/
    FadeInSampleRate=((unsigned int)FadeInDuration*250)/Intensity;
    FadeOutSampleRate=((unsigned int)FadeOutDuration*250)/Intensity;
    // </editor-fold>
    /**********************Hello world*************************/
    WriteStringOnLine(BufferLCD_1,(char*)"UFR-ST",'c',0);
    WriteStringOnLine(BufferLCD_2,(char*)"Neuro",'c',0);
    __delay_ms(15);
    BUZZER=1;
    __delay_ms(50);
    BUZZER=0;
    /**********************Loop forever*****************************************/
    while(1){//Loop forever
        if(SnapShootButton>=0x10){//Button was pushed
            switch (SnapShootButton){//Button checking
                /******************Button1********************/
                case 0x1D://Button1
                    //  <editor-fold defaultstate="collapsed" desc="Button 1">
                    switch(FlagMenu){//Menu Checking
                        case 0x10 : FlagMenu=0x60; break;
                        case 0x11 : FlagMenu=0x10; break;
                        case 0x13 :
                            FlagMenu=0x12;
                            StimulationState=0x11;//StartingFade in
                            break;
                        case 0x14 :
                            FlagMenu=0x13;
                            break;
                        case 0x20 : FlagMenu=0x10; break;
                        case 0x21 : //Increment Stim duration
                            if(StimDuration<LimSupStimDuration) {
                                StimDuration=StimDuration+10;
                            }
                            break;
                        case 0x30 : FlagMenu=0x20; break;
                        case 0x31 : //Increment of fadein
                            if(FadeInDuration<LimSupFadeInOut){
                                FadeInDuration++;
                                    }
                                    break;
                        case 0x40 : FlagMenu=0x30; break;
                        case 0x41 : //Increment of FadeOut
                            if (FadeOutDuration<LimSupFadeInOut) {
                                FadeOutDuration++;
                            }
                            break;
                        case 0x50 : FlagMenu=0x40; break;
                        case 0x51 : (Intensity<LimSupIntensity) ? Intensity++:Intensity=LimInfIntensity ; break;
                        case 0x60 : FlagMenu=0x80; break;
                        case 0x61 : 
                            FlagMenu=0x60;
                            EEPROM_WRITE(ADDR_FlagRS232Enable,FlagRS232Enable);
                            break;
                        case 0x70 : FlagMenu=0x50; break;
                        case 0x71 ://Increment ThresholdImpedance
                             if(ThresholdImpedance<LimSupThresholdImpedance){
                                ThresholdImpedance+=10;
                             }
                             break;
                        case 0x80 : FlagMenu=0x70; break;
                    }// </editor-fold>
                FlagRefreshMenu=1;
                break;//Button1
                /******************Button2********************/
                case 0x1B: //Button2
                    //  <editor-fold defaultstate="collapsed" desc="Button 2">
                    switch(FlagMenu){//Menu Checking
                        case 0x10 :FlagMenu=0x20; break;
                        case 0x12 :
                            FlagMenu=0x13;
                            StimulationState=0x13;//Ask Fade Out
                            TMR1ON=0;
                            break;
                        case 0x13 :
                            if(StimulationState==0x00){
                            FlagMenu=0x14;
                            }
                            break;
                        case 0x20 :FlagMenu=0x30; break;
                        case 0x21 ://Decrement Stim Duration
                          if(StimDuration>LimInfStimDuration) {
                                StimDuration=StimDuration-10;
                            }
                            break;
                        case 0x30 :FlagMenu=0x40; break;
                        case 0x31 ://Decrement fadein
                            if(FadeInDuration>LimInfFadeInOut){
                                FadeInDuration--;
                                    }
                                    break;
                        case 0x40 :FlagMenu=0x50; break;
                        case 0x41 ://Decrement fadeOut
                             if (FadeOutDuration>LimInfFadeInOut) {
                                FadeOutDuration--;
                            }
                            break;
                        case 0x50 :FlagMenu=0x70; break;
                        case 0x51 :(Intensity>LimInfIntensity) ? Intensity--:Intensity=LimSupIntensity; break;
                        case 0x60 :FlagMenu=0x10; break;
                        case 0x61 :
                            StimulationState=0x10;
                            FlagMenu=0x60;
                            break;
                        case 0x70 :FlagMenu=0x80; break;
                        case 0x71 ://Decrement Threshold Impedance
                            if(ThresholdImpedance>LimInfThresholdImpedance){
                               ThresholdImpedance-=10;
                                   }
                                   break;
                        case 0x80 : FlagMenu=0x60; break;
                    }// </editor-fold>
                FlagRefreshMenu=1;
                break;//Button2
                /******************Button3********************/
                case 0x17: //Button3
                    //  <editor-fold defaultstate="collapsed" desc="Button 3">
                    switch(FlagMenu){//Menu Checking
                        case 0x10 :FlagMenu=0x11; break;
                        case 0x11 ://Start stimulation
                            FlagMenu=0x12;
                            VarStimDuration=StimDuration;//Init Countdown
                            StimulationState=0x11;//StartingFade i
                            if (FlagRS232Enable==1)
                            {TXEN=1;}//start USART 
                            break;
                        case 0x14 :FlagMenu=0x10; StimulationState=0x10;
                            if (FlagRS232Enable==1)
                            {TXEN=0;}//stop USART 
                        case 0x20 :FlagMenu=0x21; break;
                        case 0x21 :
                            EEPROM_WRITE(ADDR_StimDuration_L, (char)(StimDuration&0x00FF));
                            EEPROM_WRITE(ADDR_StimDuration_H, (char)(StimDuration>>8));
                            FlagMenu=0x20;
                            FlagRefreshParameters=1;
                            break;
                        case 0x30 :FlagMenu=0x31; break;
                        case 0x31 :
                            FlagMenu=0x30;
                            EEPROM_WRITE(ADDR_FadeInDuration,FadeInDuration);
                            FlagRefreshParameters=1;
                            break;
                        case 0x40 :FlagMenu=0x41; break;
                        case 0x41 :
                            FlagMenu=0x40;
                            EEPROM_WRITE(ADDR_FadeOutDuration,FadeOutDuration);
                            FlagRefreshParameters=1;
                            break;
                        case 0x50 :FlagMenu=0x51; break;
                        case 0x51 :
                            FlagMenu=0x50;
                            EEPROM_WRITE(ADDR_Intensity,Intensity);
                            FlagRefreshParameters=1;
                            break;
                        case 0x60 :
                            VarStimDuration=0;
                            FlagMenu=0x61;
                            StimulationState=0x14;
                            break;
                        case 0x70 :FlagMenu=0x71; break;
                        case 0x71 :
                            FlagMenu=0x70;
                            EEPROM_WRITE(ADDR_ThresholdImpedance_L, (char)(ThresholdImpedance&0x00FF));
                            EEPROM_WRITE(ADDR_ThresholdImpedance_H, (char)(ThresholdImpedance>>8));
                            break;
                        case 0x80 :
                            if (FlagRS232Enable==0){
                                FlagRS232Enable=1;
                            }
                            else{
                                FlagRS232Enable=0;
                            }
                            EEPROM_WRITE(ADDR_FlagRS232Enable,FlagRS232Enable);
                            break;
                    }// </editor-fold>
                FlagRefreshMenu=1;
                break;//Button3
            }
        SnapShootButton=0;//unlock button
        }//End of button section

        /* the next section involve the displaying of menus*/
        if (FlagRefreshMenu>=1) {
            // <editor-fold defaultstate="collapsed" desc="Flag menu check and writing on LCD BUFFER">
            if (FlagRefreshMenu==1){
                DisplayClear(); 
                 Padding(BufferLCD_1,16,' ');
                 Padding(BufferLCD_2,16,' ');
            }//t=84us
            switch (FlagMenu)   {
                case 0x10://main menu
                    WriteStringOnLine(BufferLCD_1,(char*)"START       OK",'r',0);
                    WriteStringOnLine(BufferLCD_2,(char*)"  STIMULATION",'l',0);
                    break;
                case 0x11://Start Stimulation confirmation
                case 0x14://Stop Stimulation confirmation
                    (FlagMenu==0x11) ? WriteStringOnLine(BufferLCD_1,(char*)"NO   START   YES",'l',0):WriteStringOnLine(BufferLCD_1,(char*)"NO   STOP    YES",'l',0);
                    WriteStringOnLine(BufferLCD_2,(char*)"STIMULATION",'c',0);
                    break;
                case 0x12://Stimulation
                case 0x13://PauseR4
                case 0x61://Computer control
                    switch(FlagRefreshMenu++){
                        case 1 :
                            if (FlagMenu==0x61){//print 61->PC else play
                                WriteStringOnLine(BufferLCD_1,(char*)"PC",'l',0);
                            }
                            else{
                                 BufferLCD_1[0]=0x07;
                            }
                            if (FlagMenu==0x12) BufferLCD_2[0]=0x04;
                            if (FlagMenu==0x13)BufferLCD_2[0]=0x05;//print 12->pause else stop
                            break;
                        case 2 :
                             BufferLCD_1[6]='u';BufferLCD_1[7]='A';
                             UInt2Str(CaptIntensity,StrBuffer_8);
                             WriteStringOnLine(BufferLCD_1,StrBuffer_8,'r',-10);
                             break;
                        case 3 :
                            WriteFloatOnLine(BufferLCD_2,StrBuffer_8,CaptTension,1,'r',0,(char*)"V"); //print voltage
                            break;
                        case 4 :
                            strcpy(StrBuffer_4,"K");StrBuffer_4[1]=6;
                            WriteFloatOnLine(BufferLCD_2,StrBuffer_8,CaptImpedance,1,'r',-7,StrBuffer_4);//print Impedance
                            break;
                        case 5 :
                            BufferLCD_1[15]='s';
                            UInt2Str(VarStimDuration,StrBuffer_8);
                            WriteStringOnLine(BufferLCD_1,StrBuffer_8,'r',-1);//print Duration
                            FlagRefreshMenu=0;
                            FlagRefreshDisplay=2;
                            break;
                    //t=2.5msR
                    }
                    break;
                case 0x20://Stimulation time menu
                    WriteStringOnLine(BufferLCD_1,(char*)"STIM DUR.  SET",'r',0);
                    WriteNumberOnLine(BufferLCD_2,StrBuffer_8,StimDuration,'l',2,(char*)" s");
                    break;
                case 0x21://Stimulation time setup
                    WriteStringOnLine(BufferLCD_1,(char*)"+ DURATION  SAVE",'r',0);
                    WriteNumberOnLine(BufferLCD_2,StrBuffer_8,StimDuration,'l',2,(char*)" s");
                    WriteStringOnLine(BufferLCD_2,(char*)"- ",'l',0);
                    break;
                case 0x30://Fade IN
                    WriteStringOnLine(BufferLCD_1,(char*)"FADE IN    SET",'r',0);
                    WriteNumberOnLine(BufferLCD_2,StrBuffer_8,FadeInDuration,'l',2,(char*)" s");
                    break;
                case 0x31://Fade IN time setup
                    WriteStringOnLine(BufferLCD_1,(char*)"+ FADE IN   SAVE",'r',0);
                    WriteNumberOnLine(BufferLCD_2,StrBuffer_8,FadeInDuration,'l',2,(char*)" s");
                    WriteStringOnLine(BufferLCD_2,(char*)"- ",'l',0);
                    break;
                case 0x40://Fade OUT
                    WriteStringOnLine(BufferLCD_1,(char*)"FADE OUT   SET",'r',0);
                    WriteNumberOnLine(BufferLCD_2,StrBuffer_8,FadeOutDuration,'l',2,(char*)" s");
                    break;
                case 0x41://Fade OUT time setup
                    WriteStringOnLine(BufferLCD_1,(char*)"+ FADE OUT  SAVE",'r',0);
                    WriteNumberOnLine(BufferLCD_2,StrBuffer_8,FadeOutDuration,'l',2,(char*)" s");
                    WriteStringOnLine(BufferLCD_2,(char*)"- ",'l',0);
                    break;
                case 0x50://Intensity
                    WriteStringOnLine(BufferLCD_1,(char*)"INTENSITY  SET",'r',0);
                    WriteNumberOnLine(BufferLCD_2,StrBuffer_8,Intensity,'l',2,(char*)" uA");
                    break;
                case 0x51://Intensity setup
                    WriteStringOnLine(BufferLCD_1,(char*)"+ INTENSITY SAVE",'r',0);
                    WriteNumberOnLine(BufferLCD_2,StrBuffer_8,Intensity,'l',2,(char*)" uA");
                    WriteStringOnLine(BufferLCD_2,(char*)"- ",'l',0);
                    break;
                case 0x60://computer-control
                    WriteStringOnLine(BufferLCD_1,(char*)"COMP.   ENABLE",'r',0);
                    WriteStringOnLine(BufferLCD_2,(char*)"CONTROL       ",'r',0);
                    break;
                case 0x70://Threshold of impedance
                    WriteStringOnLine(BufferLCD_1,(char*)"THLD. IMP. SET",'r',0);
                    strcpy(StrBuffer_4,"K");StrBuffer_4[1]=6;
                    WriteFloatOnLine(BufferLCD_2,StrBuffer_8,ThresholdImpedance,1,'l',2,StrBuffer_4);//print Impedance;
                    break;
                case 0x71://Intensity setup
                    WriteStringOnLine(BufferLCD_1,(char*)"+ THRESHOLD SAVE",'r',0);
                    WriteFloatOnLine(BufferLCD_2,StrBuffer_8,ThresholdImpedance,1,'l',2,StrBuffer_4);//print Impedance;
                    WriteStringOnLine(BufferLCD_2,(char*)"- ",'l',0);
                    break;
                case 0x80://computer-control
                    if (FlagRS232Enable==1){
                       WriteStringOnLine(BufferLCD_1,(char*)"RS232  ENABLED",'r',0);
                    }
                    else{
                        WriteStringOnLine(BufferLCD_1,(char*)"RS232 DISABLED",'r',0);
                    }
                    WriteStringOnLine(BufferLCD_2,(char*)"9600Bds       ",'r',0);
                    break;
            }//End switch menu
                if ((FlagMenu & 0x0F) ==0){//For first level
                    BufferLCD_1[0]=0x01;BufferLCD_2[0]=0x02;//print arrow
                }
            if (FlagRefreshMenu==1){
                FlagRefreshMenu=0;
               FlagRefreshDisplay=2;
            }
        }//End refresh menu
        // </editor-fold>
        /* the next section involve the calculation and saving parameter*/
        if (FlagRefreshParameters){
            switch(FlagMenu){//Time base =1024 us
                case 0x50:/*save intensity eeprom;*/
                    FadeInSampleRate=((unsigned int)FadeInDuration*250)/Intensity;
                    FadeOutSampleRate=((unsigned int)FadeOutDuration*250)/Intensity;
                case 0x30:/*save intensity eeprom;*/
                    FadeInSampleRate=((unsigned int)FadeInDuration*250)/Intensity;
                    break;
                case 0x40:/*save intensity eeprom;*/
                    FadeOutSampleRate=((unsigned int)FadeOutDuration*250)/Intensity;
                    break;
            }
            FlagRefreshParameters=0;
        }
        if (StimulationState>0x0F){
            // <editor-fold defaultstate="collapsed" desc="execution of requests : change of stimulation state ">
            switch (StimulationState){
                case 0x10 : //request for stop stimulation
                    if (FlagMenu!=0x13){
                        TMR6IE=0; TMR6ON=0;
                    RCEN=0;
                    RCIE=0;
                    }
                    TMR1IE=0; TMR4IE=0;
                    TMR1ON=0; TMR4ON=0;
                    TMR1=0; TMR1L=0xDB; TMR1H=0x0B;
                    TMR1IE=1; TMR1ON=1;
                    StimulationState=0x00;
                    set_pwm(0);
                    //TXEN=0;
                    TX_Prefix='#';
                    break;
                case 0x11 : //request for fade in
                    CounterForTxBuffer=-1;
                    TXEN=1;
                    TMR6=0, TMR6IF=0; TMR6IE=1;
                    TMR4=0; TMR4IF=0; TMR4IE=1;
                    StimulationState=0x01;
                    TMR4SampleRate=FadeInSampleRate;//Time base 1/2024us
                    TMR4ON=1;
                    TMR6ON=1;
                    TX_Prefix='+';
                    break;
                case 0x12 : //request for stop fade in and start stimulation
                    TMR4IE=0; TMR4ON=0;
                    TMR1L=0xDB; TMR1H=0x0B;
                    TMR1IE=1; TMR1ON=1;
                    StimulationState=0x02;
                    TX_Prefix='*';
                    break;
                case 0x13 : //request for fade out
                    TMR1IE=0; TMR1ON=0;
                    TMR4=0; TMR4IF=0; TMR4IE=1;
                    StimulationState=0x03;
                    TMR4SampleRate=FadeOutSampleRate;//Time base 1/2024us
                    TMR4ON=1; 
                    TMR6ON=1;
                    TX_Prefix='-';
                    break;
                case 0x14 : //request for mode computer-controlling
                    TMR1L=0xDB; TMR1H=0x0B; TMR1IF=0; TMR1IE=1;
                    TMR6=0, TMR6IF=0; TMR6IE=1;
                    StimulationState=0x04;
                    TMR1ON=1;
                    TMR6ON=1;
                    CREN=1;
                    RCEN=1;
                    SPEN=1;
                    TXEN=1;
                    RCIE=1;
                    break;
            }
        }
        // </editor-fold>
        /* Calculation of intensity, voltage and resistor*/
        if (FlagCaptState==2){ //A2D conversion is done for intensity and voltage
            if (CaptRAWIntensity>4){
                CaptImpedance=(unsigned int)(((unsigned short long)(CaptRAWTension<<3))*143/CaptRAWIntensity);
            }
            else{
                CaptImpedance=0;
            }
            CaptTension=(CaptRAWTension<<1)/7; 
            CaptIntensity=CaptRAWIntensity>>2;
            if (ThresholdImpedance<CaptImpedance){ //Alert if impedance is to high
                BUZZER=1;
            }
            if(FlagCapt2TX_GO_DONE==1 && StimulationState==0x04){
               switch (TX_Prefix){
                   case 'v' : itoa(TX_Buffer,CaptRAWTension,10);
                   CounterForTxBuffer=-2;
                   TXIE=1;
                   break;
                   case 'i' : itoa(TX_Buffer,CaptRAWIntensity,10);
                   CounterForTxBuffer=-2;
                   TXIE=1;
                   break;
               }
               FlagCapt2TX_GO_DONE=0;
            }
            if (FlagRS232Enable==1 && StimulationState!=0x04 ){
               Num2Str(TX_Buffer,CaptImpedance,1,(char*)"KR");
               CounterForTxBuffer=-2;
               TXIE=1;
            }
            FlagCaptState=0;

        }//End of variable calculation
        if (FlagRefreshDisplay>0){
            switch(FlagRefreshDisplay){
                case 2 : FlagRefreshDisplay=1;SetDDRamAddress(0x00);DisplayCursor=0;break;
                case 1 :
                    if (DisplayCursor==16){
                        SetDDRamAddress(0x40);
                    }
                    (DisplayCursor<16) ? WriteData(BufferLCD_1[DisplayCursor]) : WriteData(BufferLCD_2[DisplayCursor-16]) ;
                    if (DisplayCursor<32){
                        DisplayCursor++;
                    }
                    else {
                        FlagRefreshDisplay=0;
                    }

            }
        }
    }//End while(1)
}//End main function