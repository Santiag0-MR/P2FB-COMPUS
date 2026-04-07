#include <xc.h>
#include "pic18f4321.h"
#include "TAD_EEPROM.h"
 

#define BASE_ANIMALS     0    // 24 animals x 4 bytes = 96 bytes (0x00 - 0x5F)
#define BASE_NUM_ANIMALS 96   // 4 bytes (0x60 - 0x63)
#define EEPROM_UNUSED    0xFF
 
/*
 * Funcions privades de baix nivell
 */
static void EEPROM_WriteByte(unsigned char addr, unsigned char data) {
    EEADR  = addr;
    EEDATA = data;
    EECON1bits.EEPGD = 0;  // Accedir a EEPROM (no Flash)
    EECON1bits.CFGS  = 0;  // Accedir a EEPROM (no Config)
    EECON1bits.WREN  = 1;  // Habilitar escriptura
    di();                  // Desactivar interrupcions durant la sequencia
    EECON2 = 0x55;         // Sequencia obligatoria
    EECON2 = 0xAA;
    EECON1bits.WR = 1;     // Iniciar escriptura
    ei();                  // Tornar a activar interrupcions
    while(EECON1bits.WR);  // Esperar fi d'escriptura (tipicament 4ms)
    EECON1bits.WREN = 0;   // Deshabilitar escriptura
}
 
static unsigned char EEPROM_ReadByte(unsigned char addr) {
    EEADR  = addr;
    EECON1bits.EEPGD = 0;  // Accedir a EEPROM
    EECON1bits.CFGS  = 0;
    EECON1bits.RD    = 1;  // Iniciar lectura
    return EEDATA;
}