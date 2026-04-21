#include <xc.h>
#include "TAD_LDR.h"
#include "TAD_SERIAL.h"
#include "TAD_TIMER.h"
#include "TAD_ADC.h"

#define TICS_5S 5000
#define LLINDAR_LDR 200

static unsigned char timerLDR;
static unsigned char sleepRecived = 0;


void LDR_Init(){
    TI_NewTimer(&timerLDR);
}

void LDR_SleepReceived(){
    sleepRecived = 1;
}

void LDR_Motor(){
    static unsigned char estado = 0;
    static unsigned char i;   
    static unsigned int valorLDR;
    
    switch(estado){
        case 0:
            if(sleepRecived){ 
                TI_ResetTics(timerLDR);
                sleepRecived = 0;
                estado++;
            }
            break;
        case 1:
            if(ADC_ExistValueLDR()){
                valorLDR = ADC_GetLDR();
                i = 0;
                estado++;
            }
            break;
        case 2:
            if(i == 7){ 
                estado++;
            }else{
                i++;
                valorLDR = valorLDR >> 1;
            }
            break;
        case 3:
            valorLDR = valorLDR & 0x07; 
            if(valorLDR < LLINDAR_LDR){
                // LDR tapado
                SERIAL_PutString(5);   // CMD_SLEEP_SUCCESSFUL
                estado = 0;
            } else if(TI_GetTics(timerLDR) >= TICS_5S){
                // LDR sin tapar
                SERIAL_PutString(6); // CMD_SLEEP_UNSUCCESSFUL
                estado = 0;
            }
            break;
    }
}

