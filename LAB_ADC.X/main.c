#include <xc.h>
#include "TAD_ADC.h"
#include "TAD_DISPLAY.h"
#include "TAD_SERIAL.h"
#include "TAD_TIMER.h"

#pragma config OSC   = HS
#pragma config PBADEN = DIG
#pragma config MCLRE = ON
#pragma config DEBUG = OFF
#pragma config PWRT  = OFF
#pragma config BOR   = OFF
#pragma config WDT   = OFF
#pragma config LVP   = OFF

static void __interrupt (high_priority) MyRSI (void);
void initPorts(void);

static void __interrupt (high_priority) MyRSI (void){
    if (TMR0IF == 1){
        RSI_Timer0();
    }
}

void initPorts(){
    TRISBbits.TRISB2 = 1;
    INTCON2bits.RBPU = 0; 
}

void main(void) {
    initPorts();
    TI_Init();
    ADC_Init();
    DSP_Init();
    SERIAL_Init();
    

    while(1){
        ADC_Motor();
        DSP_Motor();
        Motor_Serial();
    }
}