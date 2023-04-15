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
// GPIO2 signal entree a diviser par 2 si freq > 3Hz
// GPIO5 entree lampe inhibition de freq divise par 2
#define PERIODE_1_MS 61535  //0xFFFF - 4000

volatile unsigned char cmpt=0,oldgp2=0;
volatile unsigned long ms=0;
unsigned long temps;
void __interrupt() my_isr()
{
    if(PIR1bits.T1IF==1)     // si overflow
    {
        T1CONbits.TMR1ON=0;  // arret timer
        TMR1=PERIODE_1_MS;   // reglage timer
        PIR1bits.T1IF=0;     // on annule l'interruption
        ms++;                // on incrémente le compteur de ms
        //GPIObits.GP4= ~GPIObits.GP4;
        T1CONbits.TMR1ON=1;  // on redemarre le compteur
    }
}

void delay(unsigned long delais_ms)
{
    unsigned long tps;         // variable locale
    tps=ms;                    // fixée à ms
    while((ms-tps)<delais_ms); // on boucle tant que le délai n'est pas dépassé 
}

void main(void) {
    ANSEL=0;                    // pas d'analogique utilisé
    GPIO=0;                     // les bits passés à "1" sont inversés 
    TRISIO=0x24;                // placer les pattes 2 et 5 en entrée (0b100100)
    WPUbits.WPU2=1;             // pull up sur la broche 2
    INTCONbits.INTE=0;
    // Config Timer1 pour ISR toute les 3S
    CMCON1bits.T1ACS=1;         // 4MHz for timer1
    T1CONbits.TMR1CS=0;         // source horloge interne
    T1CONbits.T1CKPS=0;         // diviseur 1:1
    TMR1=PERIODE_1_MS;          // fixe la limite du compteur 
    T1CONbits.TMR1ON=1;         //Timer1 OFF
    PIE1bits.T1IE=1;            // pas trouvé dans la doc.... preparation des interruptions
    INTCONbits.PEIE=1;          // pas clair dans la doc... preparation des interruptions / compteur timer ? 
    ei();                       //interrupt enable
    oldgp2=0;
    for(;;)
    {     
        if(GPIObits.GP5==0)
        {
            GPIObits.GP4=GPIObits.GP2;
        }
        else
        {
            if(oldgp2==0)
            {
                if(GPIObits.GP2)
                {
                    oldgp2=1;
                    if((ms-temps)>320)
                    {
                        GPIObits.GP4=1;
                    }
                    else
                    {                
                        if(cmpt%2)
                        {
                            GPIObits.GP4=1;
                        }
                        else
                        {
                            GPIObits.GP4=0;
                        }
                    }
                    di();
                    temps=ms;
                    ei();
                    cmpt++;
                    delay(20);
                }
            }
            if(oldgp2==1)
            {
                if(GPIObits.GP2==0)
                {
                    oldgp2=0;
                    GPIObits.GP4=0;
                    
                    //di();
                    //temps=ms;
                    //ei();
                    //cmpt++;
                 //   delay(20);
                }
            }
        }
    }   
    return;
}



