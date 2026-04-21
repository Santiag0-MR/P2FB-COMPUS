#ifndef T_TAD_EEPROM_H
#define	T_TAD_EEPROM_H

#include <xc.h> 

void EEPROM_WriteByte(unsigned char addr, unsigned char data);
unsigned char EEPROM_WriteAvailable();
unsigned char EEPROM_ReadByte(unsigned char addr);

#endif	

