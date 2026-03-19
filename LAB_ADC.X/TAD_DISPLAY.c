#include <xc.h>
#include <proc/pic18f4321.h>
#include "TAD_ADC.h"
#include "TAD_SERIAL.h"
#include "TAD_TIMER.h"

/* Umbrales:
 *   value = 0       -> maximo hacia un lado
 *   value = 7       -> maximo hacia el otro lado
 *   value = 3 o 4   -> zona central (reposo)
 */
#define UMBRAL_MAX  7
#define UMBRAL_MIN  0
#define REBOTES 8
#define NO_MIRAR_REBOTES 0
#define MIRAR_REBOTES_SALIDA 1
#define MIRAR_REBOTES_ENTRADA 2

static unsigned int valorX;
static unsigned int valorY;
static unsigned char lastDirX = 0xFF;  // 0xFF = ninguna
static unsigned char lastDirY = 0xFF;
static unsigned char timerRebotes;
static unsigned char flagRebotes = NO_MIRAR_REBOTES;

void DSP_Init(){
    valorX = 0;
    valorY = 0;
    TI_NewTimer(&timerRebotes);
}

void DSP_Reset(void){
    
    valorX = 0;
    valorY = 0;
}

void DSP_Motor(){
    static unsigned char estado = 0;
    static unsigned char i;
    
    switch(estado){
        case 0:
            if(ADC_ExistValue()){
                valorX = ADC_GetX();
                valorY = ADC_GetY();
                i = 0;
                estado++;
            }
            break;
        case 1:
            if(i == 7){ 
                estado++;
            }else{
                i++;
                valorX = valorX >> 1;
                valorY = valorY >> 1;
            }
            break;
        case 2:
            valorX = valorX & 0x07;
            valorY = valorY & 0x07;

            if(flagRebotes == NO_MIRAR_REBOTES){
                
                if(!PORTBbits.RB2){
                    
                    TI_ResetTics(timerRebotes);
                    flagRebotes = MIRAR_REBOTES_ENTRADA;
                    
                } else {
                    estado = 0;

                    /* Eje X */
                    if(valorX == UMBRAL_MIN){
                        
                        if(lastDirX != UMBRAL_MIN){
                            SERIAL_PutString(3);
                            lastDirX = UMBRAL_MIN;
                        }
                    } else if(valorX == UMBRAL_MAX){
                        
                        if(lastDirX != UMBRAL_MAX){
                            SERIAL_PutString(2);
                            lastDirX = UMBRAL_MAX;
                        }
                    } else {
                        lastDirX = 0xFF;
                    }

                    /* Eje Y */
                    if(valorY == UMBRAL_MIN){
                        
                        if(lastDirY != UMBRAL_MIN){
                            SERIAL_PutString(1);
                            lastDirY = UMBRAL_MIN;
                        }
                    } else if(valorY == UMBRAL_MAX){
                        
                        if(lastDirY != UMBRAL_MAX){
                            SERIAL_PutString(0);
                            lastDirY = UMBRAL_MAX;
                        }
                    } else {
                        lastDirY = 0xFF;
                    }
                }

            } else if(flagRebotes == MIRAR_REBOTES_ENTRADA){
                
                if(TI_GetTics(timerRebotes) >= REBOTES){
                    if(!PORTBbits.RB2){
                        
                        SERIAL_PutString(4);
                        flagRebotes = MIRAR_REBOTES_SALIDA;
                        estado++;  
                        
                    } else {
                        
                        flagRebotes = NO_MIRAR_REBOTES;
                        estado = 0;
                        
                    }
                }
            }
            break;
        case 3:
            if(PORTBbits.RB2){
                TI_ResetTics(timerRebotes);
                estado++;
            }
            break;
        case 4:
            if(TI_GetTics(timerRebotes) >= REBOTES){
                if(PORTBbits.RB2){
                    
                    flagRebotes = NO_MIRAR_REBOTES;
                    estado = 0;
                } else {
                    
                    estado--;
                }
            }
            break;
    }
}