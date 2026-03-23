#ifndef TAD_ADC_H
#define TAD_ADC_H

void          ADC_Init(void);
void          ADC_Motor(void);
unsigned int ADC_GetX(void);
unsigned int ADC_GetY(void);
void          ADC_Start(void);
char          ADC_Finished(void);
unsigned char ADC_ExistValue();
unsigned char ADC_ExistValueLDR();
unsigned int ADC_GetLDR();

#endif