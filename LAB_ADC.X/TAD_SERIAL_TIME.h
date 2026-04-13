#ifndef TAD_SERIAL_TIME_H
#define TAD_SERIAL_TIME_H

void STIME_Init(void);
void STIME_Motor(void);
unsigned char STIME_DateRecieved(void);
void STIME_GetDate(unsigned char *day, unsigned char *month, unsigned char *hour, unsigned char *min, unsigned char *sec);

#endif