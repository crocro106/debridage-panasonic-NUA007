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
#define PERIODE_1_MS 61535  //0xFFFF - 4000 , nombre d'impulsions avant overflow pour 1 ms � 4 MHz

volatile unsigned char cmpt=0,oldgp2=0; // cmpt = compteur d'impulsions, oldgp2 = etat pr�c�dent entr�e GP2
volatile unsigned long ms=0;            // temps �coul� depuis le d�but
unsigned long temps;                    // instant du dernier front montant d�cect�
unsigned long instant_phare=0;          // instant du d�but d'appui sur phares, si 0 c'est que pas d'appui
unsigned char marche=0;                 // etat du diviseur par deux si t>320 ms; 0 inactiv�, 1 activ�
void __interrupt() my_isr()             // capture le d�bordement de pile pour cr�er un compteur de millisecondes depuis le dernier reset
{
    if(PIR1bits.T1IF==1)     // si overflow
    {
        T1CONbits.TMR1ON=0;  // arret timer
        TMR1=PERIODE_1_MS;   // reglage timer
        PIR1bits.T1IF=0;     // on annule l'interruption
        ms++;                // on incr�mente le compteur de ms
        //GPIObits.GP4= ~GPIObits.GP4;
        T1CONbits.TMR1ON=1;  // on redemarre le compteur
    }
}

void delay(unsigned long delais_ms)         // fonction d�di�e "d�lai" - ne rend la main qu'apr�s le d�lai en ms
{
    unsigned long tps;         // variable locale
    tps=ms;                    // fix�e � ms
    while((ms-tps)<delais_ms); // on boucle tant que le d�lai n'est pas d�pass� 
}

void main(void) {
    ANSEL=0;                    // pas d'analogique utilis�
    GPIO=0;                     // les bits pass�s � "1" sont invers�s 
    TRISIO=0x24;                // placer les pattes 2 et 5 en entr�e (0b100100)
    WPUbits.WPU2=1;             // pull up sur la broche 2
    INTCONbits.INTE=0;
    // Config Timer1 pour ISR toute les 3S
    CMCON1bits.T1ACS=1;         // 4MHz for timer1
    T1CONbits.TMR1CS=0;         // source horloge interne
    T1CONbits.T1CKPS=0;         // diviseur 1:1
    TMR1=PERIODE_1_MS;          // fixe la limite du compteur 
    T1CONbits.TMR1ON=1;         //Timer1 OFF
    PIE1bits.T1IE=1;            // pas trouv� dans la doc.... preparation des interruptions
    INTCONbits.PEIE=1;          // pas clair dans la doc... preparation des interruptions / compteur timer ? 
    ei();                       //interrupt enable
    oldgp2=0;                   // on fixe l'�tat pr�c�dent de GP2 � 0
    for(;;)                     // boucle infinie
    {     
        if(GPIObits.GP5==0)     // pas d'appui sur le bouton des phares
        {
            instant_phare=0;    // on r�initialise la valeur � "0" pour signifier "aucun appui"
        }
        else                    // appui sur le bouton phare
        {
            if (instant_phare==0)        // c'est le front montant de appui sur phare
            {
                di();                    // d�sactivation des interruptions (ne pas travailler � 2 sur ms) 
                instant_phare=ms;       // on note l'instant de d�but d'appui 
                ei();                    // on r�active les interruptions
            }
            else                         // le bouton phare �tait d�j� appuy�
            {
                if (ms-instant_phare>2000)  // si �a fait plus de 2s
                {
                    if (marche==0)          // si le diviseur �tait arr�t�
                    {
                        marche=1;           // on le met en marche
                    }
                    else                    // sinon 
                    {
                        marche=0;           // on l'arr�te
                    }
                    instant_phare=0;        // on r�initialise la variable pour recommencer � compter 2s si appui maintenu
                }            }
        }
        if(marche==0)                  // diviseur inactiv�
        {
            GPIObits.GP4=GPIObits.GP2; // on copie la sortie sur l'entr�e
        }
        else                           // on g�re la division par 2
        {
            if(oldgp2==0)              // � l'�tat pr�c�dent le switch �tait ferm�
            {
                if(GPIObits.GP2)       // il est � l'�tat ouvert, front montant
                {
                    oldgp2=1;                // on note qu'on est � l'�tat haut
                    if((ms-temps)>320)       // si �a fait plus de 320 ms on recopie l'�tat sur la sortie
                    {
                        GPIObits.GP4=1;
                    }
                    else                     // si �a fait moins de 320 ms
                    {                
                        if(cmpt%2)           // si le nombre d'impulsion est impair
                        {
                            GPIObits.GP4=1;  // on met la sortie � 1
                        }
                        else                 // sinon 
                        {
                            GPIObits.GP4=0;  // on la met � 0
                        }
                    }
                    di();                    // d�sactivation interruptions
                    temps=ms;                // pour lire l'instant (r�init de temps)
                    ei();                    // on r�active l'interruption
                    cmpt++;                  // on incr�mente le compteur d'impulsions
                    delay(20);               // d�lai de 20 ms pour masquer les rebonds de l'ILS
                }
            }
            if(oldgp2==1)                    // l'�tat pr�c�dent �tait � "1", 
            {
                if(GPIObits.GP2==0)          // si front descendant de GP2
                {
                    oldgp2=0;                // on note qu'on est � l'�tat bas
                    GPIObits.GP4=0;          // on place l'�tat bas sur la sortie
                    
                    delay(20);               // petit d�lai d'antirebond
                }
            }
        }
    }   
    return;
}



