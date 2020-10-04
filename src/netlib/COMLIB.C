#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include "net.h"
#include "COMlib.H"
#include "COMM.H"

Paquete paquete_in, paquete_out;        /* Paquetes emisi¢n-recepci¢n */

WORD    portbase;
BYTE    modem;              /* BOOLEAN */
int     velocidad;
WORD    COMport;



/************************** Set the port number to use */
WORD COM_SetPort(WORD Port)
{
    WORD                Offset, far *RS232_Addr;

    switch (Port)
    { /* Sort out the base address */
      case 1 : Offset = 0x0000;
                  break;
      case 2 : Offset = 0x0002;
                  break;
      case 3 : Offset = 0x0004;
                  break;
      case 4 : Offset = 0x0006;
                  break;
      default   : return (-1);
    }

    RS232_Addr = (WORD far *)MK_FP(0x0040, Offset);  /* Find out where the port is. */
    if (*RS232_Addr == NULL) return (-1);/* If NULL then port not used. */
    portbase = *RS232_Addr;              /* Otherwise set portbase      */

    return (0);
}

void SendCommandToModem(char *cad) {

    for (; *cad; cad++)
        comm_putc(*cad);
    comm_putc('\r');
    delay(1000);
}

void EndCOM()
{
    if (modem && partida_red.activa) {
        SendCommandToModem("~~~+++~~~ATH0");        /* Colgar el modem */
        SendCommandToModem("ATS0=0Z");              /* Reinicializar modem */
    }
    comm_close();
}


WORD InitCOM(WORD Port, WORD Speed)
{
    WORD irq;

    velocidad=Speed;

    if (COM_SetPort(Port)) return (1);

    if (Port==1 || Port==3) irq=4;
    else irq=3;

    Port=comm_open ( Port, Speed );
    if(!Port) return 2;


    COMport=Port;
    return (0);         /* No hubo error */
}


BYTE WaitModem()
{
    BYTE    ch;

    SendCommandToModem("ATS0=1");
//    while (!kbhit()) {
        if (comm_avail()) {
            ch=comm_getc(0);

            switch (ch) {
                case '2': return ('*'); //continue;
                case '1':
                case '5': return (0);
                case '3':
                case '6':
                case '8': return (ch);
//                default : return ('!');
            }
        }
        delay(200);
//    }
    return ('*');
}


BYTE DialModem(char *DialString)
{
    BYTE    ch;

    SendCommandToModem(DialString);
    while (!kbhit()) {
        if (comm_avail()) {
            ch=comm_getc(0);

            switch(ch) {
                case '1':
                case '5': return (0);
                case '3':
                case '6':
                case '7':
                case '8': return (ch);
//                default : return ('!');
            }
        }
        delay(200);
    }
   return ('*');  // se puls¢ una tecla
}


BYTE InitModem(char *ModemInitString, char *ModemDialString)
{
  if (!ModemDialString)
    return (1);
  if (!ModemInitString)
    ModemInitString="ATZ";

  SendCommandToModem(ModemInitString);
  SendCommandToModem("ATEVQW%C");
  switch (velocidad)
  {
    case 14400 :    SendCommandToModem("ATS37=11N"); break;
    default    :    SendCommandToModem("ATS37=0N1"); break;
  }

  if (partida_red.servidor)
    return (WaitModem());
  else
    return (DialModem(ModemDialString));
}


void InitWaitForOtherUser()
{
  COM_Send(1,MSG_SYNC,"SYNC",4);
}

WORD LoopWaitForOtherUser()
{
  if (COM_CheckPacket())
    switch(paquete_in.comando)
    {
      case MSG_SYNC:  COM_Send(1,MSG_ENDSYNC,"",0);
                      partida_red.servidor=1;
                      return (1);

            case MSG_ENDSYNC:   partida_red.servidor=0;
                                return (1);

//            default:            InitWaitForOtherUser();
  }
return(0);
}

WORD internal_packet(WORD comando)
{
  if (comando<10000)
    return 0;

  switch(comando) {
    case MSG_SYNC:        COM_Send(1,MSG_ENDSYNC,"",0);
                          partida_red.servidor=1;
                          partida_red.activa=1;
                          break;
    case MSG_ENDSYNC:     partida_red.servidor=0;
                          partida_red.activa=1;
                          break;
    case MSG_EXIT:        partida_red.servidor=1;
                          partida_red.activa=0;
                          break;
  }
  return 1;
}

WORD TestChecksum()
{
  WORD i,checksum;

  checksum=paquete_in.jugador+paquete_in.comando+paquete_in.long_buffer;
  for (i=0; i<paquete_in.long_buffer; i++)
    checksum+=paquete_in.buffer[i];
  if (checksum==paquete_in.checksum)
    return (1);
  else
    return (0);
}

void COM_Send(WORD Player,WORD Command,char *Buffer,WORD Long)
{
  WORD i;
  char *ptr;

  paquete_out.jugador=Player;
  paquete_out.comando=Command;
  paquete_out.long_buffer=Long;
  memcpy(paquete_out.buffer, Buffer, Long);
  paquete_out.checksum=Player+Command+Long;
  for (i=0; i<Long; i++)
    paquete_out.checksum+=Buffer[i];
  for (i=0, ptr=(char *) &paquete_out; i<sizeof(paquete_out); i++, ptr++)
    comm_putc(*ptr);
}


WORD COM_CheckPacket()
{
  char *ptr= (char *)&paquete_in;
  WORD x;

  if (comm_avail()>=sizeof(paquete_in))
  {
    _disable();
    for (x=0; x<sizeof(paquete_in); x++)
      *(ptr+x)=comm_getc(0);
    if (TestChecksum()) {
      if (!internal_packet(paquete_in.comando)) {
        memcpy(partida_red.datos,paquete_in.buffer,paquete_in.long_buffer);
/*
        if (partida_red.servidor)
          SRV_Packet(1,paquete_in.comando,paquete_in.buffer,paquete_in.long_buffer);
        else
          NOD_Packet(0,paquete_in.comando,paquete_in.buffer,paquete_in.long_buffer);
*/
      }
      _enable();
      return (1);
    }
    else {
//      sound(500);
//      delay(50);
//      nosound();
      comm_flush();
    }
  }
  _enable();
  return (0);
}

