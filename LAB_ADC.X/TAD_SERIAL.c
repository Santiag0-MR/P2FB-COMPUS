#include <xc.h>
#include "TAD_SERIAL.h"
#include "pic18f4321.h"
#include "TAD_CMD.h"
#include "TAD_LDR.h"
#include "TAD_EEPROM.h"


// Baudrate que faràs servir al terminal del PC
#define BAUDRATE 9600UL // Velocidad de comunicación serie
#define OSC_FREQ 10000000UL // Frecuencia del oscilador (10 MHz)

// Buffer para guardar el comando 
static unsigned char comando[30];

// Buffer circular de TX (cooperatiu)
static unsigned char estado = 0;
static unsigned char indice = 0;

//Frases a escribir.
static const char frase0[] = "MOVE_UP\r\n";
static const char frase1[] = "MOVE_DOWN\r\n";
static const char frase2[] = "MOVE_LEFT\r\n";
static const char frase3[] = "MOVE_RIGHT\r\n";
static const char frase4[] = "SELECT\r\n";
static const char frase5[] = "SLEEP_SUCCESSFUL\r\n";
static const char frase6[] = "SLEEP_UNSUCCESSFUL\r\n";

// comandos Java->PIC
static const char comando0[] = "GET_ANIMALS";
static const char comando1[] = "GET_PRODUCTS";
static const char comando2[] = "CONSUME";
static const char comando3[] = "RESET";
static const char comando4[] = "START_REBELLION";
static const char comando5[] = "STOP_REBELLION";
static const char comando6[] = "SLEEP";
static const char comando7[] = "INITIALIZE";


static const char * const comandos[] = {
    comando0, comando1, comando2, comando3, comando4, comando5, comando6, comando7
};

static const char * const frases[] = {
    frase0, frase1, frase2, frase3, frase4, frase5, frase6
};
static const char *pTxFrase = 0;   // apunta al char actual de la frase a imprimir

void SERIAL_Init(void){
    // TXSTA: SYNC=0 asincrono, TX9=0 mandar 8bits, BRGH=1 (alta velocidad), TXEN=1 habilitar transmissión.
    TXSTAbits.SYNC = 0;
    TXSTAbits.TX9  = 0;
    TXSTAbits.BRGH = 1;
    TXSTAbits.TXEN = 1;

    // RCSTA: SPEN=1 habilitar puertos en serie, RX9=0 recibir 8bits, CREN=1 habilitar recepción contínua.
    RCSTAbits.SPEN = 1;
    RCSTAbits.RX9  = 0;
    RCSTAbits.CREN = 1;

    // Baudrate generator
    // Amb BRGH=1: SPBRG = (Fosc/(16*BPS)) - 1
    // multiplicamos por unsigned char para asegurarnos de tener los 8bits de menor peso
    SPBRG = (unsigned char)((OSC_FREQ / (16UL * BAUDRATE)) - 1UL);
    SPBRGH = 0;

    // Limpiar el flag
    PIR1bits.RCIF = 0;
    estado = 1;
    
}

/** Envía un carácter directamente al registro de transmisión y pasa a modo TX */
void SERIAL_PutChar(char c){
    estado = 0;
    TXREG = (unsigned char)c;
}

/** Selecciona una frase de la tabla por índice y arranca su transmisión */
void SERIAL_PutString(unsigned char s){
    pTxFrase = frases[s];   // elegimos la frase
    estado = 0;             // pasamos a modo imprimir
}
  
/**
 * Intenta leer un byte del UART.
 * Devuelve 1 y escribe en *b si hay dato válido, 0 si no hay dato o hay error.
 */
unsigned char SERIAL_ReadByte(unsigned char *b){
    if (PIR1bits.RCIF == 0) return 0;     

    // Si hubo overrun, hay que resetear el receptor
    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }

    *b = RCREG;                          

    // Si hay framing error, puedes descartarlo
    if (RCSTAbits.FERR) return 0;

    return 1;                             
}

/**
 * Acumula un carácter en readArr con soporte de borrado (DEL).
 * También hace eco del carácter al terminal.
 */
void guardarCharArr(unsigned char aux){
    comando[indice] = aux;
    indice++;
    comando[indice] = '\0';
    
}


void SERIAL_Motor(void){
    unsigned char aux;
    
    switch(estado){
        case 0:
            if (PIR1bits.TXIF) {
                if(pTxFrase && *pTxFrase != '\0') {
                    char c = *pTxFrase++;
                    TXREG = (unsigned char)c;
                }else {
                    pTxFrase = 0;
                    estado = 1;
                }
            }
            break;
        case 1:
        if(SERIAL_ReadByte(&aux)){
            if(aux == '\r'){
                indice = 0;
                estado++;
            } else if(aux == '\n'){
                // ignorar \n
            } else {
                guardarCharArr(aux);
            }
        }
        break;
        case 2:
            if(comando[0] == 'S'){ //[S]EELP
                CMD_setCmd(0, comando);
                LDR_SleepReceived();
            }else if(comando[0] == 'R'){ //[R]ESET
                CMD_setCmd(1, comando);
            }else if(comando[0] == 'C'){ //[C]ONSUME
                CMD_setCmd(2, comando);
            }else if(comando[0] == 'R'){
                if(comando[1] == '1'){ //[R]1 = START REBELLION
                    CMD_setCmd(4, comando);
                }else{ // [R]2 = STOP REBELLION
                    CMD_setCmd(5, comando);
                }
            }else if(comando[0] == 'A'){
                CMD_setCmd(6, comando);
            }else if(comando[0] == 'P'){ //[P]RODUCTS
                CMD_setCmd(7, comando);
            }
            estado = 1;
            if(comando[0] == 'I'){
                estado++;
                indice = 0;
                CMD_setCmd(3, comando);
            }
            break;
        case 3: // Para la inicialización
            
            if(EEPROM_WriteAvailable()){
                if(comando[indice] != '\0' && indice <= 30){
                    
                    indice++;
                }
            }
            
            break;
    }
}
