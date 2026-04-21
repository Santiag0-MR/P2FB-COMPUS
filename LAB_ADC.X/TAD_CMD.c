#include <proc/pic18f4321.h>
#include "TAD_CMD.h"

static unsigned char estado = 0;
static unsigned char arr[30];

void CMD_setCmd(unsigned char e, unsigned char aux[30]){
    estado = e;
}

void init(){
    static unsigned char eInit = 0;
    static unsigned char indice = 1;
    
    switch(eInit){
        case 0:
            
            break;
        case 1:
            break;
    }
}

void CMD_Motor(){
    
    switch(estado){
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
    }
}




