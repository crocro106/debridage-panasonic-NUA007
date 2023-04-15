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


volatile unsigned char cmpt=0;

void __interrupt() my_isr()
{
    if(GPIObits.GP5==0)
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
    }
    else
    {
        GPIObits.GP4= ~GPIObits.GP4;
        //OPTION_REGbits.INTEDG= ~OPTION_REGbits.INTEDG; // on inverse la polarite l'ISR pour copie la pin // pas besoin on a un FET en CO en sortie sur entree pull up cote moteur
    }
    INTCONbits.INTF=0;
    SLEEP();
    NOP();
}


void main(void) {
    ANSEL=0;
    GPIO=0;
    TRISA=0x24;
    INTCONbits.INTE=1;
    ei(); //interrupt enable
    SLEEP();
    NOP();
    for(;;)
    {
    }
    return;
}
