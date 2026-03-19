#ifndef TAD_SERIAL_H
#define TAD_SERIAL_H

// Inicialitza 
void SERIAL_Init(void);

// Encola un caràcter per enviar (no bloqueja)
void SERIAL_PutChar(char c);

// Encola un string (no bloqueja)
void SERIAL_PutString(unsigned char s);

// Motor del TAD: buida la cua quan el HW està llest (cridar sovint al while(1))
void Motor_Serial(void);

void SERIAL_denegarEscritura();

void SERIAL_permitirEscritura();

#endif
