#include <xc.h>
#include "TAD_SERIAL.h"
#include "pic18f4321.h"


// Baudrate que faràs servir al terminal del PC
#define BAUDRATE 9600UL // Velocidad de comunicación serie
#define OSC_FREQ 10000000UL // Frecuencia del oscilador (10 MHz)

// Flags de comandos recibidos
static unsigned char cmdGetAnimals    = 0;
static unsigned char cmdGetProducts   = 0;
static unsigned char cmdConsume       = 0;
static unsigned char cmdReset         = 0;
static unsigned char cmdStartRebel    = 0;
static unsigned char cmdStopRebel     = 0;
static unsigned char cmdSleep         = 0;
static unsigned char cmdInitialize    = 0;

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

unsigned char SERIAL_SleepReceived(){
    if(cmdSleep == 1){
        cmdSleep = 0;
        return 1;
    }
    return 0;
}

unsigned char SERIAL_GetAnimalsReceived(){
    if(cmdGetAnimals == 1){
        cmdGetAnimals = 0;
        return 1;
    }
    return 0;
}

unsigned char SERIAL_GetProductsReceived(){
    if(cmdGetProducts == 1){
        cmdGetProducts = 0;
        return 1;
    }
    return 0;
}

unsigned char SERIAL_ResetReceived(){
    if(cmdReset == 1){
        cmdReset = 0;
        return 1;
    }
    return 0;
}

unsigned char SERIAL_StartRebellionReceived(){
    if(cmdStartRebel == 1){
        cmdStartRebel = 0;
        return 1;
    }
    return 0;
}

unsigned char SERIAL_StopRebellionReceived(){
    if(cmdStopRebel == 1){
        cmdStopRebel = 0;
        return 1;
    }
    return 0;
}

unsigned char SERIAL_ConsumeReceived(){
    if(cmdConsume == 1){
        cmdConsume = 0;
        return 1;
    }
    return 0;
}

unsigned char SERIAL_InitializeReceived(){
    if(cmdInitialize == 1){
        cmdInitialize = 0;
        return 1;
    }
    return 0;
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
                cmdSleep = 1;
            }else if(comando[0] == 'R'){ //[R]ESET
                cmdReset = 1;
            }else if(comando[0] == 'C'){ //[C]ONSUME
                cmdConsume = 1;
            }else if(comando[0] == 'I'){ //[I]NICIALIZE
                cmdInitialize = 1;
            }else if(comando[0] == 'S'){
                if(comando[2] == 'A'){ //[S]T[A]RT_REBELLION
                    cmdStartRebel = 1;
                }else{ // [S]T[O]P_REBELLION
                    cmdStopRebel = 1;
                }
            }else if(comando[0] == 'G'){
                if(comando[4] == 'A'){ //[G]ET_[A]NIMALS
                    cmdGetAnimals = 1;
                }else{ // [G]ET_[P]RODUCTS
                    cmdGetProducts = 1;
                }
            }
            estado = 1;
            break;
    }
}
