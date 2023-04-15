/*
 * File:   main.c
 * Author: xgaltier
 *
 * Created on 1 juin 2021, 19:52
 */

// PIC12HV615 Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config IOSCFS = 4MHZ    // Internal Oscillator Frequency Select (4 MHz)
#pragma config BOREN = OFF      // Brown-out Reset Selection bits (BOR disabled)

#include <xc.h>

// GPIO4 sortie freq divise par 2
// GPIO2 ( INT ) signal interruption a divise par 2 si freq > 3Hz
// GPIO5 entree lampe inhibition de fre divise par 2
#define PERIODE_3S 0x5D3C  //0xFFFF - 41666

volatile unsigned char cmpt=0,sec3occur=0;

void __interrupt() my_isr()
{
    if(PIR1bits.T1IF==1)
    {
        T1CONbits.TMR1ON=0;
        PIR1bits.T1IF=0;   
        sec3occur=1;
    }
    if((GPIObits.GP5==1)&&(sec3occur==0))
    {
        cmpt++;
        if(cmpt%2)
        {
            GPIObits.GP4=1;
        }
        else
        {
            GPIObits.GP4=0;
        }
        TMR1=PERIODE_3S;
        T1CONbits.TMR1ON=1;
        sec3occur=0;
    }
    else if(INTCONbits.INTF==1)
    {
        TMR1=PERIODE_3S;
        sec3occur=0;
        T1CONbits.TMR1ON=1;
        GPIObits.GP4= ~GPIObits.GP4;
        OPTION_REGbits.INTEDG= ~OPTION_REGbits.INTEDG; // on inverse la polarite l'ISR pour copie la pin
    }
    INTCONbits.INTF=0;
}


void main(void) {
    ANSEL=0;
    GPIO=0;
    TRISIO=0x24;
    INTCONbits.INTE=1;
    // Config Timer1 pour ISR toute les 3S
    CMCON1bits.T1ACS=0;  // 1MHz for timer1
    T1CONbits.TMR1CS=0;
    T1CONbits.T1CKPS=3;
    TMR1=PERIODE_3S;
    T1CONbits.TMR1ON=0; //Timer1 OFF
    PIE1bits.T1IE=1;
    INTCONbits.PEIE=1;        
    ei(); //interrupt enable
    for(;;)
    {
    }
    return;
}

