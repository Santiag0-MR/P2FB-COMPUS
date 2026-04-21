#ifndef TAD_SERIAL_H
#define TAD_SERIAL_H

void SERIAL_Init(void);
void SERIAL_PutChar(char c);
void SERIAL_PutString(unsigned char s);
void SERIAL_Motor(void);

unsigned char* SERIAL_GetPayload(void);

#endif