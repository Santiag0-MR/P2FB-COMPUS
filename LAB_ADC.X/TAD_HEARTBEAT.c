#include <proc/pic18f4321.h>
#include "TAD_TIMER.h"
#include "TAD_HEARTBEAT.h"

#define MS500 500
#define PERIODE_TOTAL 500

static unsigned char timerDutyCycle;
static unsigned char dutyCycle[4] = {50, 400, 50, 250};
static unsigned char indice = 0;
static unsigned char estado = 1;
static unsigned char flag = 0;


void HREATBEAT_Init(){
    TI_NewTimer(&timerDutyCycle);
}

void HEARTBEAT_setEstadoAlarma(){
    estado = 0;
}

void HEARTBEAT_clearEstadoAlarma(){
    estado = 1;
    indice = 0; 
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
            break;
        case 2:
            if(TI_GetTics(timerDutyCycle) >= dutyCycle[indice]){
                LATAbits.LA3 = 0;
                indice++;
            }
            if(TI_GetTics(timerDutyCycle) >= MS500){
                estado = 1;
            }
            break;
    }
}

