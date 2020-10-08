#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <i86.h>
#include <limits.h>
#include <time.h>
#include "ipxlib.h"
#include "net.h"
#include "dpmi_net.h"

#define NET_JOIN        10231
#define NET_ACCEPTJOIN  10232
#define NET_EXIT        10233
#define NET_SERVER      10234
#define NET_ADDUSER     10235
#define NET_LOOKINGFOR  10236

VOID SendAck(WORD NECB);
WORD WaitAck();
net_game_info game_list[32];
net_game_info game_info;

//=============================================================================
//  Zona de variables publicas de las rutinas bajo nivel.
//=============================================================================

WORD             IPX_Socket;
RealPointer      IPX_EventControlBlocks[MAX_PACK];
RealPointer      IPX_Packets[MAX_PACK];
ECB  far        *IPX_ECB[MAX_PACK];
Packet  far     *IPX_Packet[MAX_PACK];
LocalNodeAddress IPX_Broadcast;
LocalNodeAddress IPX_MyAddress;
LocalNodeAddress IPX_WorkAddress;
LocalNodeAddress IPX_WWorkAddress;

//=============================================================================
//  Zona de variables publicas de las rutinas alto nivel.
//=============================================================================

LocalNodeAddress        IPX_Users[MAX_USERS];
int                     IPX_TimeOut[MAX_USERS];
DWORD                   AckTime[MAX_USERS];
DWORD                   MyAckTime;
DWORD                   TimerCnt;               // Contador de Paquetes
WORD                    IPX_FinSyn;
WORD                    Ack;
WORD                    GlobalTimeOut;
WORD                    CntToRead;

//=============================================================================
//  Chequea si las IPX est n cargadas
//  Entrada: Nada
//  Salida : 1 si estan cargadas
//           0 si no estan cargadas
//=============================================================================

WORD IPX_CheckIfLoad()
{
  RMREGS  mregs;

  memset(&mregs,0,sizeof(mregs));
  mregs.w.AX = 0x7A00;
  int386Extend(0x2F, &mregs);

  if ((mregs.w.AX&0xFF)==0xFF)
    return 1;
  return 0;
}

//=============================================================================
//  Abre un canal de comunicacion a traves de IPX
//  Entrada: Numero de canal a abrir.
//  Salida : 1 si se pudo abrir.
//           0 si no se pudo.
//=============================================================================

WORD IPX_OpenSocket(WORD socket)
{
  RMREGS  mregs;

  memset(&mregs,0,sizeof(mregs));
  mregs.w.BX = 0;
  mregs.w.AX = 0;
  mregs.w.DX = socket;
  int386Extend(0x07A,&mregs);

  if (mregs.b.AL==0) {
    IPX_Socket=socket;
    return 1;
  }
  return 0;
}

//=============================================================================
//  Cierra un canal de comunicacion a traves de IPX
//  Entrada: Nada.
//  Salida : Nada.
//=============================================================================

VOID IPX_CloseSocket()
{
  RMREGS  mregs;

  memset(&mregs,0,sizeof(mregs));
  mregs.w.BX = 1;
  mregs.w.DX = IPX_Socket;
  int386Extend(0x07A, &mregs);
}

//=============================================================================
//  Reserva la memoria baja necesaria para las operaciones de red
//  Salida : 1 si se reservo memoria.
//           0 si no reservo.
//=============================================================================

WORD IPX_ReserveLowMemory()
{
  WORD x;

  //---------------------------------------------------------------------------
  // Reserva memoria para los paquetes
  //---------------------------------------------------------------------------
  for (x=0;x<MAX_PACK;x++) {
    IPX_ECB[x]   =( ECB far *)allocate_dos_memory(&IPX_EventControlBlocks[x],sizeof(ECB));
    IPX_Packet[x]=( Packet far *)allocate_dos_memory(&IPX_Packets[x],sizeof(Packet));
  }

  //---------------------------------------------------------------------------
  // Obtiene las direcciones fisicas
  //---------------------------------------------------------------------------
  IPX_GetInternalAddress(&IPX_MyAddress);
  IPX_GetInternalAddress(&IPX_WorkAddress);
  IPX_GetInternalAddress(&IPX_Broadcast);
  memset(IPX_Broadcast.NodeAddress,0xFF,6);
  return 1;
}

//=============================================================================
//  Libera la memoria baja necesaria para las operaciones de red
//  Entrada: Nada.
//  Salida : Nada.
//=============================================================================

VOID IPX_FreeLowMemory()
{
WORD x;

  for(x=0;x<MAX_PACK;x++)
  {
    free_dos_memory(&IPX_EventControlBlocks[x]);
    free_dos_memory(&IPX_Packets[x]);
  }
}

//=============================================================================
//  Copia la direccion de un nodo.
//  Entrada: Puntero al destino de copia.
//           Puntero al origen de copia.
//  Salida : Nada.
//=============================================================================

VOID IPX_SetNode(LocalNodeAddress far *Dest,LocalNodeAddress far *Source)
{
  FarCopy(Dest->NetWorkAddress,Source->NetWorkAddress,4);
  FarCopy(Dest->NodeAddress,Source->NodeAddress,6);
}

//=============================================================================
//  Pregunta la direccion de un nodo de red a la tarjeta
//  Entrada: Puntero al destino de copia.
//  Salida : Nada.
//=============================================================================

VOID IPX_GetInternalAddress(LocalNodeAddress *NetWorkNode)
{
  RMREGS  mregs;
  RealPointer             NWNode;
  LocalNodeAddress far    *NWkNode;

  NWkNode=( LocalNodeAddress far *)allocate_dos_memory(&NWNode,sizeof(LocalNodeAddress));

  memset(&mregs,0,sizeof(mregs));
  mregs.w.BX = 9;
  mregs.w.ES  = NWNode.Segment;
  mregs.w.SI = 0;
  int386Extend(0x07A, &mregs);

  IPX_SetNode(NetWorkNode,NWkNode);
  free_dos_memory(&NWNode);
}

//=============================================================================
//  Pone a la escucha un paquete y comprueba si llego alguno.
//  Entrada: Numero de paquete.
//  Salida : 0 si no ha llegado nada por este paquete
//           1 si ha llegado nada por este paquete
//=============================================================================

WORD IPX_ListendPacket(WORD NECB)
{
RMREGS mregs;

  if(IPX_ECB[NECB]->InUseFlag)    return((WORD)0);
  if(!IPX_Packet[NECB]->Atendido) return((WORD)1);

  IPX_Packet[NECB]->Atendido  = 0;
  IPX_Packet[NECB]->Comando   = 0;

  memset(&mregs,0,sizeof(mregs));
  mregs.w.BX = 4;
  mregs.w.ES  = IPX_EventControlBlocks[NECB].Segment;
  mregs.w.SI = 0;

  int386Extend(0x07A, &mregs);

return((WORD)0);
}

//=============================================================================
//  Manda un paquete a la direccion especificada.
//  Nota   : EL PAQUETE A MANDAR SIEMPRE ES EL CERO.
//  Entrada: Direccion donde se enviara el paquete.
//  Salida : 0 si no ha podido enviar el paquete.
//           1 si ha podido enviarlo.
//=============================================================================

WORD IPX_SendPacketAddress(LocalNodeAddress *NetWorkNode)
{
RMREGS  mregs;
WORD Cnt=0;

  do {
    while(IPX_ECB[0]->InUseFlag) {};
    FarCopy(IPX_ECB[0]->InmediateLocalAddress,NetWorkNode->NodeAddress, 6);
    memset(&mregs,0,sizeof(mregs));
    mregs.w.BX = 3;
    mregs.w.ES  = IPX_EventControlBlocks[0].Segment;
    mregs.w.SI = 0;
    int386Extend(0x07A, &mregs);
  } while( (WaitAck()) && (++Cnt!=2));

  if (Cnt==4) return 0;
  return 1;
}

//=============================================================================
//  Manda un paquete a todos los nodos de la red.
//  Nota   : EL PAQUETE A MANDAR SIEMPRE ES EL CERO.
//  Entrada: Nada.
//  Salida : 0 si no ha podido enviar el paquete.
//           1 si ha podido enviarlo.
//=============================================================================

WORD IPX_BroadCastPacket()
{
  return (IPX_SendPacketAddress(&IPX_Broadcast));
}

//=============================================================================
//  Finaliza un Event Control Block
//  Entrada: Socket de Comunicacion.
//  Salida : Si se pudo Inicializar las IPX.
//=============================================================================

WORD EndECB(WORD NECB)
{
RMREGS mregs;

  memset(&mregs,0,sizeof(mregs));
  mregs.w.BX = 6;
  mregs.w.ES = IPX_EventControlBlocks[NECB].Segment;
  mregs.w.SI = 0;
  int386Extend(0x07A, &mregs);
  return(mregs.w.AX);
}

//=============================================================================
//  Rutinas para facilitar todo el tema de Comunicaciones.
//=============================================================================

//=============================================================================
//  Inicializa un Event Control Block por defecto.
//  Entrada: Numero de ECB.
//  Salida : Nada.
//=============================================================================

VOID InitECB(ECB far *MyECB,WORD WhatPack)
{
Packet far *MPacket=IPX_Packet[WhatPack];

  _fmemset(MyECB, 0, sizeof(ECB));
  _fmemset(MPacket, 0, sizeof(Packet));

  MyECB->SocketNumber=IPX_Socket;
  MyECB->FragmentCount=1;
  MyECB->fAddress[0]=0;
  MyECB->fAddress[1]=IPX_Packets[WhatPack].Segment;
  MyECB->fSize=sizeof(Packet);

  MPacket->Atendido=1;
  MPacket->Header.PacketType=0x04;
  MPacket->Header.SourceSocket=IPX_Socket;
  MPacket->Header.DestinationSocket=IPX_Socket;
  MPacket->Time_day=ULONG_MAX;
  _fmemcpy(MPacket->Datos,"PATATA",6);
  _fmemcpy(MPacket->Colador,"IPX",4);
  MPacket->Comando=0;
}

//=============================================================================
//  Facilita la Inicializacion de todos los Event Control Blocks.
//  Entrada: Nada.
//  Salida : Nada.
//=============================================================================

VOID InitECBs()
{
  WORD i;

  TimerCnt=0;
  for (i=0;i<MAX_PACK;i++)
  {
    InitECB(IPX_ECB[i],i);
    IPX_ECB[i]->CompletCode=1;
    IPX_SetNode(&IPX_Packet[i]->Header.SourceNode,&IPX_MyAddress);
    IPX_SetNode(&IPX_Packet[i]->Header.DestinationNode,&IPX_Broadcast);
  }
  IPX_ECB[0]->CompletCode=0;
}

//=============================================================================
//  Inserta en el array de direcciones de usuario una direccion si no
//  existe.
//  Entrada: Direccion a Insertar.
//  Salida : Identificador del nuevo usuario
//=============================================================================

WORD InsertIfNot(LocalNodeAddress *NewAddress)
{
  WORD i;

  if (memcmp(IPX_MyAddress.NodeAddress,NewAddress->NodeAddress,6)==0) {
    return -1;
  }

  for (i=0;i<partida_red.max_players;i++)
  {
    if (net_players[i] && memcmp(IPX_Users[i].NodeAddress,NewAddress->NodeAddress,6)==0)
      return partida_red.num_players;
  }
  for (i=0;i<partida_red.max_players;i++)
  {
    if (net_players[i]==0)
    {
      partida_red.num_players++;
      net_players[i]=1;
      IPX_TimeOut[i]=0;
      IPX_SetNode(&IPX_Users[i],NewAddress);
      return i;
    }
  }
  return -1;
}

//=============================================================================
//  Inserta en una posicion concreta del array de direcciones de usuario
//  una direccion.
//  Entrada:  Direccion a Insertar
//            Posicion
//  Salida : Numero de usuarios registrados.
//=============================================================================

WORD InsertIn(LocalNodeAddress *NewAddress, int posicion)
{
//  WORD i;

  if (posicion>=partida_red.max_players)
    return 0;

  if (net_players[posicion])
    return 0;

  partida_red.num_players++;
  net_players[posicion]=1;
  IPX_TimeOut[posicion]=0;
  IPX_SetNode(&IPX_Users[posicion],NewAddress);
  return 1;
}

//=============================================================================
//  Borra del array de direcciones de usuario una direccion si existe.
//  Entrada: Direccion a Borrar.
//=============================================================================

void delete_player(LocalNodeAddress *NewAddress)
{
  WORD i;

  for (i=0;i<partida_red.max_players;i++) {
    if (net_players[i] && memcmp(IPX_Users[i].NodeAddress,NewAddress->NodeAddress,6)==0) {
      net_players[i]=0;
// COMOR_FER
//      memcpy((char *)((int)partida_red.datos+i*partida_red.longitud_datos),0,partida_red.longitud_datos);
      memset((char *)((int)partida_red.datos+i*partida_red.longitud_datos),0,partida_red.longitud_datos);
      partida_red.num_players--;
      return;
    }
  }
}

//=============================================================================
//  Busca en el array de direcciones al usuario una direccion.
//  Entrada: Direccion a Buscar.
//  Salida : -1 si no se encontro.
//           Numero de registro si lo encuentra.
//=============================================================================

WORD LookFor( LocalNodeAddress *NewAddress )
{
  WORD i;

  for (i=0;i<partida_red.num_players;i++)
    if (memcmp(IPX_Users[i].NodeAddress,NewAddress->NodeAddress,6)==0)
      return i;
  return -1;
}

//=============================================================================
//  Manda un paquete a un Jugador registrado en el array de direcciones
//  Entrada: Numero de Jugador.
//  Salida : Si se pudo mandar el paquete.
//=============================================================================

WORD SendPacket(WORD Player)
{
  IPX_Packet[0]->Time_day=TimerCnt++;
  return (IPX_SendPacketAddress(&IPX_Users[Player]));
}

//=============================================================================
//  Compone un paquete con los datos de entrada y se lo manda al jugador
//  Entrada: Jugador que recibe el paquete.
//           Comando a mandar al jugador.
//           Buffer a mandar.
//           Longitud del Buffer.
//  Salida : Si se pudo mandar el paquete.
//=============================================================================

WORD SendPacketToPlayer(WORD Player,WORD Command,char *Buffer,WORD Lengt)
{
//  int i;

  IPX_Packet[0]->Atendido=0;
  IPX_Packet[0]->Comando=Command;
  IPX_Packet[0]->Longitud=Lengt;
  _fmemcpy(IPX_Packet[0]->Datos,Buffer,Lengt);
  return(SendPacket(Player));
}

//=============================================================================
//  Compone un paquete con los datos de entrada y se lo manda a todos los
//  jugadores
//  Entrada: Comando a mandar a los jugadores.
//           Buffer a mandar.
//           Longitud del Buffer.
//  Salida : Nada
//=============================================================================

VOID SendPacketToAll(WORD Command,char *Buffer,WORD Lengt)
{
  int i;

  IPX_Packet[0]->Atendido=0;
  IPX_Packet[0]->Comando=Command;
  IPX_Packet[0]->Longitud=Lengt;
  _fmemcpy(IPX_Packet[0]->Datos,Buffer,Lengt);
  for (i=0;i<partida_red.max_players;i++) {
    if (net_players[i])
      SendPacket(i);
  }
}

//=============================================================================
//  Inicializa las IPX.
//  Entrada: Socket de Comunicacion.
//  Salida : Si se pudo Inicializar las IPX.
//=============================================================================

WORD InitIPX(WORD socket)
{
int x;

  Ack=0;
  TimerCnt=0;
  partida_red.num_players=0;

  //---------------------------------------------------------------------------
  // Chequea si las IPX est n cargadas
  //---------------------------------------------------------------------------
  if (!IPX_CheckIfLoad())
    return 0;

  //---------------------------------------------------------------------------
  // Abre un canal de comunicacion a traves de IPX
  //---------------------------------------------------------------------------
  if (!IPX_OpenSocket(socket))
    return 0;

  //---------------------------------------------------------------------------
  // Reserva la memoria baja necesaria para las operaciones de red
  //---------------------------------------------------------------------------
  IPX_ReserveLowMemory();

  for (x=0;x<MAX_USERS;x++)
    AckTime[x]=0;
  MyAckTime=0;
  return 1;
}

//=============================================================================
//  Finaliza las IPX.
//  Entrada: Nada.
//  Salida : Nada.
//=============================================================================

VOID EndIPX()
{
  WORD i;

  //---------------------------------------------------------------------------
  // Finaliza los Event Control Block
  //---------------------------------------------------------------------------
  for (i=1;i<MAX_PACK;i++)
    EndECB(i);

  //---------------------------------------------------------------------------
  // Cierra un canal de comunicacion a traves de IPX
  //---------------------------------------------------------------------------
  IPX_CloseSocket();

  //---------------------------------------------------------------------------
  // Libera la memoria baja necesaria para las operaciones de red
  //---------------------------------------------------------------------------
  IPX_FreeLowMemory();
}

//=============================================================================
//  Realiza una busqueda de paquetes en los ECB para sincronismo.
//  Entrada: Nada
//  Salida : Numero de ECB donde se recibe el paquete
//=============================================================================

WORD CheckLookPackets()
{
  WORD i;//,j;
//  static char WBuffer[200];

  for (i=1;i<MAX_PACK;i++)
  {
    if (IPX_ListendPacket(i))
    {
      IPX_Packet[i]->Atendido=1;
      _fmemcpy(IPX_WorkAddress.NodeAddress,IPX_ECB[i]->InmediateLocalAddress,6);
      //-----------------------------------------------------------------------
      // Si el mensaje no lo he enviado yo, me lo quedo
      //-----------------------------------------------------------------------
      if (_fmemcmp(IPX_ECB[i]->InmediateLocalAddress,IPX_MyAddress.NodeAddress,6))
        return i;
    }
  }
return 0;
}

//=============================================================================
//  Realiza una busqueda de paquetes en los ECB para sincronismo.
//  Entrada: Nada
//  Salida : Numero de ECB donde se recibe el paquete
//=============================================================================

WORD check_command(int comando)
{
WORD i;

  for (i=1;i<MAX_PACK;i++)
  {
    if (IPX_ListendPacket(i))
    {
      if (IPX_Packet[i]->Comando==comando)
      {
        return i;
      }
    }
  }
return 0;
}

//=============================================================================
//  Realiza una busqueda de paquetes en los ECB para sincronismo.
//  Entrada: Nada
//  Salida : Numero de ECB donde se recibe el paquete
//=============================================================================

void delete_command(int comando)
{
WORD i;
  for (i=1;i<MAX_PACK;i++)
  {
    if (IPX_ListendPacket(i))
    {
      if (IPX_Packet[i]->Comando==comando)
      {
        IPX_Packet[i]->Atendido=1;
      }
    }
  }
}

//=============================================================================
//  Comprueba si hay partidas activas en la red.
//  Salida : Numero de partidas activas
//=============================================================================

int looking_for_games(int timeout)
{
  WORD i,j;
//  int x,y;
//  char Buffer[100];
  int num_games=0,dir_ok=0;
  time_t time_counter=clock();
//  int contador=1000;

  timeout*=CLOCKS_PER_SEC;

  while (1) { //contador-->0)
    IPX_Packet[0]->Atendido=0;
    IPX_Packet[0]->Comando=NET_LOOKINGFOR;
    IPX_Packet[0]->Longitud=0;
    IPX_BroadCastPacket();
    delete_command(NET_LOOKINGFOR);
    if (j=check_command(partida_red.game_id))
    {
      _fmemcpy(IPX_WorkAddress.NodeAddress,IPX_ECB[j]->InmediateLocalAddress,6);
      _fmemcpy(&game_list[num_games].direccion,&IPX_WorkAddress,sizeof(LocalNodeAddress));
      _fmemcpy(&game_info,IPX_Packet[j]->Datos,IPX_Packet[j]->Longitud);
      strcpy(game_list[num_games].nombre,game_info.nombre);
      strcpy(net_games[num_games],game_info.nombre);
      game_list[num_games].player_id=game_info.player_id;
      dir_ok=1;
      for (i=0;i<num_games;i++) {
        if (!stricmp(game_list[i].nombre,game_list[num_games].nombre)) {
          dir_ok=0;
        }
      }
      if (dir_ok) {
        num_games++;
      }
      delete_command(partida_red.game_id);
      if (num_games==32)
        break;
    }
    //-----------------------------------------------------------------------
    // Timeout
    //-----------------------------------------------------------------------
    if (clock()-time_counter>timeout) {
      break;
    }
  }
  return num_games;
}

//=============================================================================
//  Comprueba si hay partidas activas en la red.
//  Salida : Numero de partidas activas
//=============================================================================

int join_game(int num_game)
{
  WORD j;//,i;
//  int x,y;
//  char Buffer[100];
  time_t time_counter=clock();
  time_t timeout=5*CLOCKS_PER_SEC;

  while (1)
  {
    IPX_Packet[0]->Atendido=0;
    IPX_Packet[0]->Longitud=0;
    IPX_Packet[0]->Comando=NET_JOIN;
    _fmemcpy(IPX_WorkAddress.NodeAddress,game_list[num_game].direccion.NodeAddress,6);
    IPX_SendPacketAddress(&IPX_WorkAddress);
    if (partida_red.activa)
      return (partida_red.player_id); // No ha habido errores
    if (j=check_command(NET_ACCEPTJOIN))
    {
      _fmemcpy(IPX_WorkAddress.NodeAddress,IPX_ECB[j]->InmediateLocalAddress,6);
      _fmemcpy(&game_info,IPX_Packet[j]->Datos,IPX_Packet[j]->Longitud);
      strcpy(partida_red.nombre,game_info.nombre);
      partida_red.game_id=game_info.game_id;
//      partida_red.num_players=game_info.num_players;
      partida_red.max_players=game_info.max_players;
      partida_red.player_id=game_info.player_id;
      partida_red.activa=1;
      InsertIn(&IPX_WorkAddress,game_list[num_game].player_id);
      InsertIn(&IPX_MyAddress,partida_red.player_id);
      delete_command(NET_ACCEPTJOIN);
      return (partida_red.player_id); // No ha habido errores
    }

    //-----------------------------------------------------------------------
    // Timeout
    //-----------------------------------------------------------------------
    if (clock()-time_counter>timeout)
    {
      break;
    }
  }
return -1; // Se ha producido un error
}

//=============================================================================
//  Sale de la partida actual
//=============================================================================

int ipx_exit_game()
{
  WORD i,j;

  for (i=0;i<10;i++)
  {
    if (partida_red.servidor)
    {
      IPX_Packet[0]->Atendido=0;
      IPX_Packet[0]->Longitud=0;
      IPX_Packet[0]->Comando=NET_SERVER;
      for (j=0;j<partida_red.num_players;j++)
      {
        if (net_players[j] && j!=partida_red.player_id)
        {
          SendPacket(j);
          break;
        }
      }
    }
    IPX_Packet[0]->Atendido=0;
    IPX_Packet[0]->Longitud=0;
    IPX_Packet[0]->Comando=NET_EXIT;
    for (j=0;j<partida_red.num_players;j++)
    {
      SendPacket(j);
    }
    delay(10);
  }
  partida_red.servidor=0;
  return 0;
}

//=============================================================================
//  Busca al ultimo
//=============================================================================

WORD LookForOldest()
{
DWORD MaxTime;
WORD i,OLDESTECB;

  OLDESTECB=0;
  MaxTime=ULONG_MAX;
  for (i=1;i<MAX_PACK;i++)
    if (!IPX_ECB[i]->InUseFlag && !IPX_Packet[i]->Atendido)
      if(IPX_Packet[i]->Time_day < MaxTime)
      {
        MaxTime=IPX_Packet[i]->Time_day;
        OLDESTECB=i;
      }
return(OLDESTECB);
}

//=============================================================================
//  Busca al primero
//=============================================================================

WORD LookForNewest()
{
  DWORD MaxTime;
  WORD i,NEWESTECB;

  NEWESTECB=0;
  MaxTime=0;
  for (i=1;i<MAX_PACK;i++)
    if (!IPX_ECB[i]->InUseFlag && !IPX_Packet[i]->Atendido)
      if (IPX_Packet[i]->Time_day > MaxTime)
      {
        MaxTime=IPX_Packet[i]->Time_day;
        NEWESTECB=i;
      }
      else
      {
        IPX_Packet[i]->Atendido=1;
      }
return(NEWESTECB);
}

//=============================================================================
//
//=============================================================================

void check_timeout(LocalNodeAddress *IPX_WorkAddress)
{
  return;
/*FER
int player,i,j;
  if (IPX_WorkAddress==NULL) {
    for (i=0;i<partida_red.num_players;i++) {
      IPX_TimeOut[i]++;
      if (IPX_TimeOut[i]>=1000) {
        net_players[i]=0;
        partida_red.num_players--;
      }
    }
    return;
  }

  player=LookFor(IPX_WorkAddress);
  if (player!=-1) {
    IPX_TimeOut[player]=0;
  }
*/
}

//=============================================================================
//  Chequea paquetes
//=============================================================================

WORD CheckPackets()
{
WORD i;//,j;
static char WBuffer[200];
WORD OLDESTECB;

  check_timeout(NULL);
  GlobalTimeOut=0;
// Vacia las demas!!!!!
  if ( !( OLDESTECB=LookForOldest() ) )
//  if ( !( OLDESTECB=LookForNewest() ) )
  {
    if ( Ack )
    {
      CntToRead++;
      if (CntToRead==2000)  GlobalTimeOut=1;
      delay(1);
    }
    if (partida_red.num_players>=partida_red.max_players)
      return 0;

    for (i=1;i<MAX_PACK;i++)
    {
      if ( IPX_ListendPacket(i) )
      {
        //---------------------------------------------------------------------
        // Si el mensaje no lo he enviado yo, me lo quedo
        //---------------------------------------------------------------------
        if (_fmemcmp(IPX_ECB[i]->InmediateLocalAddress,IPX_MyAddress.NodeAddress,6))
        {
          OLDESTECB=i;
          break;
        }
        else
        {
          IPX_Packet[i]->Atendido=1;
        }
      }
    }
    if (!OLDESTECB)
      return 0;
  }

  CntToRead=0;
  _fmemcpy(IPX_WorkAddress.NodeAddress,IPX_ECB[OLDESTECB]->InmediateLocalAddress,6);
  if (_fmemcmp(IPX_ECB[OLDESTECB]->InmediateLocalAddress,IPX_MyAddress.NodeAddress,6))
  {
    if (!_fmemcmp(IPX_Packet[OLDESTECB]->Colador,"IPX",4))
    {
      check_timeout(&IPX_WorkAddress);
      _fmemcpy(WBuffer,IPX_Packet[OLDESTECB]->Datos,IPX_Packet[OLDESTECB]->Longitud);
      // Se recibe un paquete?, pues se manda ACK.
      SendAck(OLDESTECB);
      SendAck(OLDESTECB);
      if (GetInternalPacket(OLDESTECB,&IPX_WorkAddress))
      {
        memcpy((char *)((int)partida_red.datos+IPX_Packet[OLDESTECB]->Comando*partida_red.longitud_datos),WBuffer,IPX_Packet[OLDESTECB]->Longitud);
      }
      IPX_Packet[OLDESTECB]->Atendido=1;
      IPX_Packet[OLDESTECB]->Time_day=ULONG_MAX;
      IPX_ListendPacket(OLDESTECB);
      return 1;
    }
  }
  IPX_Packet[OLDESTECB]->Atendido=1;
  IPX_Packet[OLDESTECB]->Time_day=ULONG_MAX;
  IPX_ListendPacket(OLDESTECB);
  return 0;
}

//=============================================================================
//  Recoge un paquete interno
//=============================================================================

DWORD GetInternalPacket(WORD OLDESTECB,LocalNodeAddress *IPX_WorkAddress)
{
  int i;
  char buffer[32];
  WORD Comando=IPX_Packet[OLDESTECB]->Comando;

  if ( Comando<partida_red.max_players )
  {
    if (LookFor(IPX_WorkAddress)==-1)
      return 0;
    else
      return 1;
  }

  switch(Comando)
  {
    case NET_SERVER:
      partida_red.servidor=1;
      delete_player(IPX_WorkAddress);
      break;
    case NET_EXIT:
      delete_player(IPX_WorkAddress);
      break;
    case NET_ADDUSER:
      _fmemcpy(IPX_WorkAddress,IPX_Packet[OLDESTECB]->Datos,sizeof(LocalNodeAddress));
      if (InsertIn(IPX_WorkAddress,IPX_Packet[OLDESTECB]->Datos[10])) {
        _fmemcpy(buffer,&IPX_MyAddress,sizeof(LocalNodeAddress));
        buffer[10]=(char)partida_red.player_id;
        for (i=0;i<10;i++) {
          SendPacketToPlayer(IPX_Packet[OLDESTECB]->Datos[10],NET_ADDUSER,buffer,11);
          delay(5);
        }
      }
      break;
    case NET_JOIN:
      if (partida_red.num_players>=partida_red.max_players ||
          !partida_red.servidor)
        break;
      _fmemcpy(buffer,IPX_WorkAddress,sizeof(LocalNodeAddress));
      game_info.player_id=InsertIfNot(IPX_WorkAddress);
      IPX_Packet[0]->Longitud=sizeof(net_game_info);
      IPX_Packet[0]->Comando=NET_ACCEPTJOIN;
      strcpy(game_info.nombre,partida_red.nombre);
      game_info.game_id=partida_red.game_id;
      game_info.num_players=partida_red.num_players;
      game_info.max_players=partida_red.max_players;
      _fmemcpy(IPX_Packet[0]->Datos,&game_info,sizeof(net_game_info));
      for (i=0;i<10;i++) {
        IPX_SendPacketAddress(IPX_WorkAddress);
      }
      buffer[10]=(char)game_info.player_id;
      for (i=0;i<10;i++) {
        SendPacketToAll(NET_ADDUSER,buffer,11);
        delay(10);
      }
      ClearECBS();
      break;
/*
    case NET_ACCEPTJOIN:
      _fmemcpy(&game_info,IPX_Packet[OLDESTECB]->Datos,IPX_Packet[OLDESTECB]->Longitud);
      strcpy(partida_red.nombre,game_info.nombre);
      partida_red.game_id=game_info.game_id;
      partida_red.max_players=game_info.max_players;
      partida_red.player_id=game_info.player_id;
      partida_red.activa=1;
      InsertIn(IPX_WorkAddress,game_list[0].player_id);
      InsertIn(&IPX_MyAddress,partida_red.player_id);
      delete_command(NET_ACCEPTJOIN);
//      return (partida_red.player_id); // No ha habido errores
      break;
*/
    case NET_LOOKINGFOR:
      if (partida_red.num_players>=partida_red.max_players ||
          !partida_red.servidor)
        break;
      IPX_Packet[0]->Atendido=0;
      IPX_Packet[0]->Comando=partida_red.game_id;
      IPX_Packet[0]->Longitud=sizeof(net_game_info);
      strcpy(game_info.nombre,partida_red.nombre);
      game_info.player_id=partida_red.player_id;
      game_info.game_id=partida_red.player_id;
      game_info.num_players=partida_red.num_players;
      game_info.max_players=partida_red.max_players;
      _fmemcpy(IPX_Packet[0]->Datos,&game_info,sizeof(net_game_info));
      for (i=0;i<10;i++) {
        IPX_SendPacketAddress(IPX_WorkAddress);
      }
      delete_command(NET_LOOKINGFOR);
      break;
  }
  return 0;
}

//=============================================================================
//  Muestra la direccion interna
//=============================================================================

BYTE *ShowInternalAddress(LocalNodeAddress far *NetWorkNode)
{
static BYTE InternDir[22];

        sprintf((char *)InternDir,"%02X%02X%02X%02X-",
                                              NetWorkNode->NetWorkAddress[0],
                                              NetWorkNode->NetWorkAddress[1],
                                              NetWorkNode->NetWorkAddress[2],
                                              NetWorkNode->NetWorkAddress[3]);
        sprintf((char *)&InternDir[9],"%02X%02X%02X%02X%02X%02X",
                                        NetWorkNode->NodeAddress[0],
                                        NetWorkNode->NodeAddress[1],
                                        NetWorkNode->NodeAddress[2],
                                        NetWorkNode->NodeAddress[3],
                                        NetWorkNode->NodeAddress[4],
                                        NetWorkNode->NodeAddress[5]);
return(InternDir);
}

//=============================================================================
//  Manda el ACK
//=============================================================================

VOID SendAck(WORD NECB)
{
RMREGS  mregs;
  if( !Ack )  return;
  while(IPX_ECB[0]->InUseFlag){};
  FarCopy(IPX_ECB[0]->InmediateLocalAddress,IPX_ECB[NECB]->InmediateLocalAddress, 6);
  IPX_Packet[0]->Comando=MSG_ACK;
  IPX_Packet[0]->Atendido=0;
  memset(&mregs,0,sizeof(mregs));
  mregs.w.BX = 3;
  mregs.w.ES  = IPX_EventControlBlocks[0].Segment;
  mregs.w.SI = 0;
  int386Extend(0x07A, &mregs);
}

//=============================================================================
//  Espera el ACK
//=============================================================================

WORD WaitAck()
{
WORD Exit=0;
WORD TimeOut=0;
RMREGS  mregs;
WORD ReturnCode=1;
WORD x;
  if( !Ack )  return 0;
  while(!Exit)
  {
    for(x=1;x<MAX_PACK;x++)
      if((!IPX_ECB[x]->InUseFlag)&&(!IPX_Packet[x]->Atendido)
          &&(IPX_Packet[x]->Comando==MSG_ACK))
          {
            ReturnCode=0;
            Exit=1;
          }
          delay(100);
          if(TimeOut++==1000) Exit=1;
  }
  if(ReturnCode==0)
    for(x=1;x<MAX_PACK;x++)
      if((IPX_Packet[x]->Comando==MSG_ACK) && (!IPX_ECB[x]->InUseFlag) )
      {
        IPX_Packet[x]->Atendido=0;
        IPX_Packet[x]->Comando=0;
        memset(&mregs,0,sizeof(mregs));
        mregs.w.BX = 4;
        mregs.w.ES  = IPX_EventControlBlocks[x].Segment;
        mregs.w.SI = 0;
        int386Extend(0x07A, &mregs);
      }
return(ReturnCode);
}

//=============================================================================
//  Vacia los ECBS
//=============================================================================

void ClearECBS()
{
RMREGS  mregs;
WORD x;
  for(x=1;x<MAX_PACK;x++)
    if( !IPX_ECB[x]->InUseFlag )
    {
      IPX_Packet[x]->Atendido =0;
      IPX_Packet[x]->Comando  =0;
      memset(&mregs,0,sizeof(mregs));
      mregs.w.BX = 4;
      mregs.w.ES = IPX_EventControlBlocks[x].Segment;
      mregs.w.SI = 0;
      int386Extend(0x07A, &mregs);
    }
}

