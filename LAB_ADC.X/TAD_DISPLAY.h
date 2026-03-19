#ifndef TAD_DISPLAY_H
#define TAD_DISPLAY_H

void DSP_Init(void);
void DSP_Show(unsigned char numToShow);
void DSP_Incrementa(void);
void DSP_Reset(void);

void DSP_SetLeft(void);
void DSP_ClearLeft(void);
void DSP_SetRight(void);
void DSP_ClearRight(void);
void DSP_SetUp(void);
void DSP_ClearUp(void);
void DSP_SetDown(void);
void DSP_ClearDown(void);
void DSP_Motor();
#endif