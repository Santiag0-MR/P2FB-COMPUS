#include <proc/pic18f4321.h>
#include "TAD_TIMER.h"
#include "TAD_HEARTBEAT.h"

#define PERIODE_TOTAL 20

static unsigned char timerDutyCycle;
static unsigned char dutyCycle = 0;
static unsigned char estado = 1;
static unsigned char flag = 0;


void HREATBEAT_Init(){
    TI_NewTimer(&timerDutyCycle);
}

void HEARTBEAT_setEstadoAlarma(){
    estado = 0;
}

void HEARTBEAT_clearEstadoAlarma(){
    estado = 0;
}

void HEARTBEAT_Motor(){
    
    switch(estado){
        case 0:
            LATAbits.LA3 = 0;
            break;
        case 1:
            TI_ResetTics(timerDutyCycle);
            LATAbits.LA3 = 1;
            estado++;
            if(dutyCycle == 40){
                flag = 1;
            }
            if(dutyCycle == 0){
                flag = 0;
            }
            if(flag == 0){
                dutyCycle++;
            }else{
                dutyCycle--;
            }
            break;
        case 2:
            if(TI_GetTics(timerDutyCycle) >= dutyCycle){
                LATAbits.LA3 = 0;
            }
            if(TI_GetTics(timerDutyCycle) >= PERIODE_TOTAL){
                estado = 1;
            }
            break;
    }
}

