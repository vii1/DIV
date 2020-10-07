#ifndef __COMLIB_H
#define __COMLIB_H

typedef struct _Paquete {
    char    jugador;
    WORD    comando;
    char    long_buffer;
    char    buffer[64];     ///// DATA BUFFER. Increase if needed!!!!!!!!!!
    WORD    checksum;
} Paquete;

extern  Paquete paquete_in, paquete_out;    /* Paquetes emisi¢n-recepci¢n */

extern  WORD    portbase;
extern  int     velocidad;

extern  BYTE    modem;              /* BOOLEAN */

/*- Funciones de Protocolos --------------------------------------*/

WORD InitCOM(WORD Port, WORD Speed);    /* para modem y serie */
BYTE InitModem(char *ModemInitString, char *ModemDialString);
    /* si hay m¢dem, hay que llamarla despu‚s de InitCOM */
void EndCOM();                          /* para modem y serie */

void InitWaitForOtherUser(void);      /* solo para cable serie */
WORD LoopWaitForOtherUser();      /* solo para cable serie */

WORD COM_CheckPacket(void);
WORD COM_CheckPacketin(void);
void COM_Send(WORD Player, WORD Command, char *Buffer, WORD Long);
void COM_Send2(WORD Player, char Command, char *Buffer, WORD Long);


#ifndef FALSE
    #define FALSE           0
#endif
#ifndef TRUE
    #define TRUE            1
#endif



#endif
