#ifndef TAD_SERIAL_H
#define TAD_SERIAL_H

void SERIAL_Init(void);
void SERIAL_PutChar(char c);
void SERIAL_PutString(unsigned char s);
void Motor_Serial(void);

unsigned char SERIAL_SleepReceived(void);
unsigned char SERIAL_GetAnimalsReceived(void);
unsigned char SERIAL_GetProductsReceived(void);
unsigned char SERIAL_ResetReceived(void);
unsigned char SERIAL_StartRebellionReceived(void);
unsigned char SERIAL_StopRebellionReceived(void);
unsigned char SERIAL_ConsumeReceived(void);
unsigned char SERIAL_InitializeReceived(void);

unsigned char* SERIAL_GetPayload(void);

#endif