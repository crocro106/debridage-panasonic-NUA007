/*
 * File:   main.c
 * Author: xgaltier / modif L. Granjon
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
#define PERIODE_1_MS 61535  //0xFFFF - 4000 , nombre d'impulsions avant overflow pour 1 ms à 4 MHz

volatile unsigned char cmpt=0,oldgp2=0; // cmpt = compteur d'impulsions, oldgp2 = etat précédent entrée GP2
volatile unsigned long ms=0;            // temps écoulé depuis le début
unsigned long temps;                    // instant du dernier front montant décecté
unsigned long instant_phare=0;          // instant du début d'appui sur phares, si 0 c'est que pas d'appui
unsigned char marche=0;                 // etat du diviseur par deux si t>320 ms; 0 inactivé, 1 activé
void __interrupt() my_isr()             // capture le débordement de pile pour créer un compteur de millisecondes depuis le dernier reset
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

void delay(unsigned long delais_ms)         // fonction dédiée "délai" - ne rend la main qu'après le délai en ms
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
    oldgp2=0;                   // on fixe l'état précédent de GP2 à 0
    for(;;)                     // boucle infinie
    {     
        if(GPIObits.GP5==0)     // pas d'appui sur le bouton des phares
        {
            instant_phare=0;    // on réinitialise la valeur à "0" pour signifier "aucun appui"
        }
        else                    // appui sur le bouton phare
        {
            if (instant_phare==0)        // c'est le front montant de appui sur phare
            {
                di();                    // désactivation des interruptions (ne pas travailler à 2 sur ms) 
                instant_phare=ms;       // on note l'instant de début d'appui 
                ei();                    // on réactive les interruptions
            }
            else                         // le bouton phare était déjà appuyé
            {
                if (ms-instant_phare>2000)  // si ça fait plus de 2s
                {
                    if (marche==0)          // si le diviseur était arrêté
                    {
                        marche=1;           // on le met en marche
                    }
                    else                    // sinon 
                    {
                        marche=0;           // on l'arrête
                    }
                    instant_phare=0;        // on réinitialise la variable pour recommencer à compter 2s si appui maintenu
                }            }
        }
        if(marche==0)                  // diviseur inactivé
        {
            GPIObits.GP4=GPIObits.GP2; // on copie la sortie sur l'entrée
        }
        else                           // on gère la division par 2
        {
            if(oldgp2==0)              // à l'état précédent le switch était fermé
            {
                if(GPIObits.GP2)       // il est à l'état ouvert, front montant
                {
                    oldgp2=1;                // on note qu'on est à l'état haut
                    if((ms-temps)>320)       // si ça fait plus de 320 ms on recopie l'état sur la sortie
                    {
                        GPIObits.GP4=1;
                    }
                    else                     // si ça fait moins de 320 ms
                    {                
                        if(cmpt%2)           // si le nombre d'impulsion est impair
                        {
                            GPIObits.GP4=1;  // on met la sortie à 1
                        }
                        else                 // sinon 
                        {
                            GPIObits.GP4=0;  // on la met à 0
                        }
                    }
                    di();                    // désactivation interruptions
                    temps=ms;                // pour lire l'instant (réinit de temps)
                    ei();                    // on réactive l'interruption
                    cmpt++;                  // on incrémente le compteur d'impulsions
                    delay(20);               // délai de 20 ms pour masquer les rebonds de l'ILS
                }
            }
            if(oldgp2==1)                    // l'état précédent était à "1", 
            {
                if(GPIObits.GP2==0)          // si front descendant de GP2
                {
                    oldgp2=0;                // on note qu'on est à l'état bas
                    GPIObits.GP4=0;          // on place l'état bas sur la sortie
                    
                    delay(20);               // petit délai d'antirebond
                }
            }
        }
    }   
    return;
}



