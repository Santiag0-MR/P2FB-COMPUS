#include <xc.h>
#include <proc/pic18f4321.h>
#include "TAD_ADC.h"
#include "TAD_DISPLAY.h"
#include "TAD_SERIAL.h"
#include "TAD_TIMER.h"
#include "TAD_LDR.h"
#include "TAD_HEARTBEAT.h"

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
    ADCON0 = 0x01;  // canal AN0, ADC encendido
    ADCON1 = 0x0C;  // AN0, AN1, AN2 anal�gicos, resto digital
    ADCON2 = 0x80;  // right-justified
    TRISAbits.TRISA3 = 0;

    TRISBbits.TRISB3 = 1;   // RB3 = entrada (RX del canal Serial_Time)
    TRISAbits.TRISA4 = 0;   // RA4 = salida (TX del canal Serial_time)
}

void main(void) {
    initPorts();
    TI_Init();
    ADC_Init();
    DSP_Init();
    SERIAL_Init();
    LDR_Init();
    HREATBEAT_Init();


    while(1){
        ADC_Motor();
        DSP_Motor();
        Motor_Serial();
        HEARTBEAT_Motor();
    }
}