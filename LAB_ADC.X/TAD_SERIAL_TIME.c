#include <xc.h>
#include "TAD_SERIAL.h"
#include "TAD_SERIAL_TIME.h"
#include "TAD_TIMER.h"

#define DELAY_FULL_BIT 260 // delay bit entero
#define DELAY_HALF_BIT 160 // delay medio bit

static unsigned char rxBuf[15];  // DD/MM HH:MM:SS = 14 chars + \r
static unsigned char rxIdx = 0;
static unsigned char dateReady = 0;
static unsigned char parseDay, parseMonth;
static unsigned char parseHour, parseMin, parseSec;

static unsigned char t_siot;
static unsigned char t_siot_rx;

static const char * const string[2] = {"Date and time correct\r\n", "Please input a correct date\r\n"};
static unsigned char ptxChar = 0;

static unsigned char validarData (unsigned char *buf) {
   // Comprobar longitud y posiciones de los separadores: DD/MM HH:MM:SS
   if (buf[2] != '/' || buf[5] != ' ' || buf[8] != ':' || buf[11] != ':') return 0;

      parseDay = (rxBuf[0] - '0') * 10 + (rxBuf[1] - '0');
      parseMonth = (rxBuf[3] - '0') * 10 + (rxBuf[4] - '0');
      parseHour = (rxBuf[6] - '0') * 10 + (rxBuf[7] - '0');
      parseMin = (rxBuf[9] - '0') * 10 + (rxBuf[10] - '0');
      parseSec = (rxBuf[12] - '0') * 10 + (rxBuf[13] - '0');

      if (parseDay < 1 || parseDay > 31) return 0;
      if (parseMonth < 1 || parseMonth > 12) return 0;
      if (parseHour > 23) return 0;
      if (parseMin > 59) return 0;
      if (parseSec > 59) return 0;
      return 1;
}

unsigned char putCharTime() {
   static unsigned char bitIdx = 0;
   static unsigned char estado_tx = 0;

   if (TI_GetTics(t_siot) >= 2) {
      unsigned char currChar = string[dateReady][ptxChar];

      if (currChar == '\0') {
         ptxChar = 0;
         bitIdx = 0;
         estado_tx = 0;
         return 1;
      }

      switch (estado_tx) {
         case 0: // ESTADO: START BIT
            LATAbits.LATA4 = 0;
            estado_tx = 1;
            bitIdx = 0;
            break;

         case 1: // ESTADO: DATA BITS (8 bits)
            LATAbits.LATA4 = (currChar >> bitIdx) & 0x01;
            bitIdx++;
            if (bitIdx >= 8) {
               estado_tx = 2;
            }
            break;

         case 2: // ESTADO: STOP BIT
            LATAbits.LATA4 = 1;
            estado_tx = 0;
            ptxChar++;
            break;
      }

      TI_ResetTics(t_siot);
   }
   return 0;
}

unsigned char getCharTime(unsigned char *c) {
   static unsigned char bitIdx_rx = 0;
   static unsigned char estado_rx = 0;
   static unsigned char byteTemporal = 0;

   switch(estado_rx) {
      case 0: // ESPERANDO BIT DE INICIO
         if (PORTBbits.RB3 == 0) {
            TI_ResetTics(t_siot_rx);
            estado_rx = 1;
         }
         break;

      case 1: // ESPERAR 1.5 BITS PARA LEER EL CENTRO DEL BIT 0
         // Si 1 bit = 2 tics, 1.5 bits = 3 tics
         if (TI_GetTics(t_siot_rx) >= 3) {
            TI_ResetTics(t_siot_rx);

            // Leer el bit y guardarlo
            if (PORTBbits.RB3) byteTemporal |= (1 << 0);
            else byteTemporal &= ~(1 << 0);

            bitIdx_rx = 1;
            estado_rx = 2;
         }
         break;

      case 2: // LEER LOS BITS RESTANTES (CENTRO DE CADA BIT)
         if (TI_GetTics(t_siot_rx) >= 2) { // 2 tics = 1 bit de distancia
            TI_ResetTics(t_siot_rx);

            if (PORTBbits.RB3) {
               byteTemporal |= (1 << bitIdx_rx);
            } else  {
               byteTemporal &= ~(1 << bitIdx_rx);
            }

            bitIdx_rx++;

            if (bitIdx_rx >= 8) {
               *c = byteTemporal;
               estado_rx = 3; // Ir a esperar Bit de Parada
            }
         }
         break;

      case 3: // BIT DE PARADA (STOP BIT)
         if (TI_GetTics(t_siot_rx) >= 2) {
            estado_rx = 0; // Volver al inicio
            return 1;      // Indicar que tenemos un nuevo carácter
         }
         break;
   }
   return 0;
}

void STIME_Init(void) {
    TI_NewTimer(&t_siot);
    TI_NewTimer(&t_siot_rx);
}

void STIME_Motor(void){
   static unsigned char estado = 0;
   unsigned char c;

   switch (estado){
   case 0:
      if (getCharTime(&c)) {
         if (c == '\r'){
            rxBuf[rxIdx] = '\0';
            rxIdx = 0;
            estado++;
         } else if (rxIdx <= 13) {
            rxBuf[rxIdx] = c;
            rxIdx++;
         } else {
            rxIdx = 0;
            estado++;
         }
      }
      break;

   case 1:  // Validar y parsear: formato DD/MM HH:MM:SS
      dateReady = validarData(rxBuf) ? 0 : 1;
      estado++;
      ptxChar = 0;
      TI_ResetTics(t_siot);
      break;
   case 2:
      estado = putCharTime() ? 0 : 2;
      break;
   }
}
