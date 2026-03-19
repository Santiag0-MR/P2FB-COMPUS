#include <xc.h>
#include "TAD_SERIAL.h"
#include "pic18f4321.h"


// Baudrate que faràs servir al terminal del PC
#define BAUDRATE 9600UL // Velocidad de comunicación serie
#define OSC_FREQ 10000000UL // Frecuencia del oscilador (10 MHz)
#define CHAR_BORRAR 127 // Código ASCII del carácter DEL

// Buffer circular de TX (cooperatiu)
#define SZ 100 // Tamaño máximo del buffer de lectura
static unsigned char estado = 0;
static unsigned char readArr[SZ]; // Buffer donde se acumula lo que escribe el usuario
static unsigned char indice = 0; // Posición actual de escritura en readArr
static unsigned char enviarCR = 0; // Flag: indica que falta enviar el '\r' tras un '\n'
static unsigned char permisosEscritura = 0; // Flag: 1 = el usuario puede escribir, 0 = bloqueado

//Frases a escribir.
static const char frase0[] = "MOVE_UP\r\n";
static const char frase1[] = "MOVE_DOWN\r\n";
static const char frase2[] = "MOVE_LEFT\r\n";
static const char frase3[] = "MOVE_RIGHT\r\n";
static const char frase4[] = "SELECT\r\n";



static const char * const frases[] = {
    frase0, frase1, frase2, frase3, frase4
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
    
    SERIAL_denegarEscritura();
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
    
    if(aux == CHAR_BORRAR){
        if(indice > 0){
            indice--;
            readArr[indice] = '\0';
        }
    }else{
        readArr[indice] = aux;
        indice++;
        readArr[indice] = '\0';
    }
    TXREG = aux;
    
}

/** Permite al usuario escribir por el terminal */
void SERIAL_permitirEscritura(){
    permisosEscritura = 1;
}

/** Bloquea la escritura del usuario*/
void SERIAL_denegarEscritura(){
    permisosEscritura = 0;
}

/**
 * Motor_Serial ? Gestiona la transmisión y recepción serie de forma cooperativa.
 *
 * Estado 0 (TX): Envía la frase apuntada por pTxFrase carácter a carácter.
 *                Convierte '\n' en la secuencia '\n'+'\r' que esperan los terminales.
 *                Al terminar la frase, pasa al estado 1.
 *
 * Estado 1 (RX): Lee lo que escribe el usuario y lo acumula en readArr.
 *                Al pulsar Enter ('\r'), notifica a PIN y EXIT que hay respuesta
 *                lista para validar, resetea el índice y envía un salto de línea.
 */
void Motor_Serial(void){
    unsigned char aux;
    
    
    switch(estado){
        case 0:
            if (PIR1bits.TXIF) {

                // Si tenemos pendiente enviar el '\r'
                if(pTxFrase && *pTxFrase != '\0') {

//                    char c = *pTxFrase++;
//
//                    if (c == '\n') {
//                        TXREG = '\n';     // primero enviamos \n
//                        enviarCR = 1;     // luego enviaremos \r
//                    }else {
//                        TXREG = (unsigned char)c;
//                    }
                    char c = *pTxFrase++;
                    TXREG = (unsigned char)c;
                }else {
                    pTxFrase = 0;
                    estado = 1;
                }
            }
            break;
        case 1:
            if(SERIAL_ReadByte(&aux) && permisosEscritura == 1){
                if(aux == '\r'){
//                    setFlagCheckRespuesta();
//                    EXIT_setFlagCheckRespuestaExit();
                    indice = 0;
                    SERIAL_PutString(12);
                }else{
                    guardarCharArr(aux);
                }
            }
            break;
    }
}
