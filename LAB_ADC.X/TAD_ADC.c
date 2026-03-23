#include <xc.h>

static unsigned char estat;
static unsigned int valueX;   // eje horizontal (AN0)
static unsigned int valueY;   // eje vertical   (AN1)
static unsigned int valueLDR;   // LDR   (AN2)
static unsigned char dataReady = 0;
static unsigned char dataReadyLDR = 0; 


void ADC_Init(){
    estat  = 0;
    valueX = 0;
    valueY = 0;
}

unsigned char ADC_ExistValue(){
    return dataReady == 1;
}

unsigned char ADC_ExistValueLDR(){
    return dataReadyLDR;
}

unsigned int ADC_GetLDR(){
    return valueLDR;
}

unsigned int ADC_GetX(void){
    return valueX;
}

unsigned int ADC_GetY(void){
    return valueY;
}

void ADC_Start(void){
    ADCON0bits.GO_DONE = 1;
}

char ADC_Finished(void){
    return !ADCON0bits.GO_DONE;
}

/*
 * Maquina de estados:
 *   0 - iniciar conversion eje X (AN0)
 *   1 - esperar fin X, leer, cambiar a AN1
 *   2 - iniciar conversion eje Y (AN1)
 *   3 - esperar fin Y, leer, volver a AN0
 */
void ADC_Motor(){
    switch(estat){
        case 0:
            ADCON0bits.CHS3 = 0;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS1 = 0;
            ADCON0bits.CHS0 = 0;  // seleccionar AN0
            estat++;
            dataReady = 0;
            ADC_Start();
            break;
        case 1:
            if(ADC_Finished()){
                valueX = ((unsigned int)ADRESH * 256) | ADRESL;
                //ADC_CalculateValue(&valueX);
                estat++;
            }
            break;
        case 2:
            ADCON0bits.CHS3 = 0;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS1 = 0;
            ADCON0bits.CHS0 = 1;  // seleccionar AN1
            estat++;
            dataReady = 0;
            ADC_Start();
            break;
        case 3:
            if(ADC_Finished()){
                valueY = ((unsigned int)ADRESH * 256) | ADRESL;
                //ADC_CalculateValue(&valueY);
                dataReady = 1;
                estat++;
            }
            break;
        case 4:
            ADCON0bits.CHS3 = 0;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS1 = 1;
            ADCON0bits.CHS0 = 0;  // AN2
            dataReadyLDR = 0;
            ADC_Start();
            estat++;
            break;
        case 5:
            if(ADC_Finished()){
                valueLDR = ((unsigned int)ADRESH * 256) | ADRESL;
                dataReadyLDR = 1;
                estat = 0;
            }
            break;
    }
}