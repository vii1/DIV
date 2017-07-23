#ifndef __IPXLIB_H
#define __IPXLIB_H

#include "DPMI_NET.H"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long  DWORD;
typedef unsigned short WORD;
typedef unsigned char  BYTE;
typedef char  far *    BYTEP;
typedef void           VOID;

/*---------------------------------*/
#define MAX_PACK        22
#define MAX_USERS       16

/*---------------------------------*/

typedef struct _LocalNodeAddress
{
        BYTE                    NetWorkAddress[4];
        BYTE                    NodeAddress[6];
}LocalNodeAddress;

/*---------------------------------*/

typedef struct { LocalNodeAddress direccion;
                 char nombre[32];
                 int player_id;
                 int game_id;
                 int num_players;
                 int max_players;
               } net_game_info;

extern net_game_info game_list[32];

/*---------------------------------*/

typedef struct _IPXHeader
{
        WORD                    CheckSum;
        WORD                    LengthInBytes;
        BYTE                    TrasportControl;
        BYTE                    PacketType;
        LocalNodeAddress        DestinationNode;
        WORD                    DestinationSocket;
        LocalNodeAddress        SourceNode;
        WORD                    SourceSocket;
}IPXHeader;

/*---------------------------------*/

typedef struct _Packet
{
        IPXHeader       Header;
        WORD            Atendido;
        BYTE            Colador[4];
        DWORD           Comando;
        BYTE            Destino;
        DWORD           Time_day;
        WORD            Longitud;
        BYTE            Datos[400];
}Packet;

/*---------------------------------*/

typedef struct _FragmentDescriptor
{
        BYTEP                   FragmentDir;
        WORD                    FragmentSize;
}FragmentDescriptor;

/*---------------------------------*/

typedef struct _EventControlBlock
{
        WORD                    Link[2];
        WORD                    EventServiceRoutine[2];
        BYTE                    InUseFlag;
        BYTE                    CompletCode;
        WORD                    SocketNumber;
        BYTE                    IPXWorkSpace[4];
        BYTE                    DriverWorkSpace[12];
        BYTE                    InmediateLocalAddress[6];
        WORD                    FragmentCount;
        WORD                    fAddress[2];            /* offset-segment */
        WORD                    fSize;                  /* low-high */

}ECB;

/*---------------------------------*/

typedef struct _NewUsers{
        WORD Number;
        WORD NewUsers[4];
        }NewUsers;

/***************************************************************************/
/* Variables para las Rutinas de Bajo nivel.                               */
/***************************************************************************/
extern WORD             IPX_Socket;
extern RealPointer      IPX_EventControlBlocks[MAX_PACK];
extern RealPointer      IPX_Packets[MAX_PACK];
extern ECB  far        *IPX_ECB[MAX_PACK];
extern Packet  far     *IPX_Packet[MAX_PACK];
extern LocalNodeAddress IPX_MyAddress;
extern LocalNodeAddress IPX_Broadcast;
extern LocalNodeAddress IPX_WorkAddress;
extern LocalNodeAddress IPX_WWorkAddress;

/***************************************************************************/
/* Rutinas de bajo nivel para Comunicaciones por IPX.                      */
/***************************************************************************/
// mira si las ipx estan cargadas.
WORD IPX_CheckIfLoad();
// Abre el canal de comunicaciones.
WORD IPX_OpenSocket(WORD socket);
VOID IPX_CloseSocket();
WORD IPX_ReserveLowMemory();
VOID IPX_FreeLowMemory();
VOID IPX_SetNode(LocalNodeAddress far *Dest,LocalNodeAddress far *Source);
VOID IPX_GetInternalAddress(LocalNodeAddress *NetWorkNode);
WORD IPX_ListendPacket(WORD NECB);
WORD IPX_SendPacketAddress(LocalNodeAddress *NetWorkNode);
WORD IPX_BroadCastPacket();
WORD EndECB(WORD NECB);
/***************************************************************************/
/* Variables para facilitar todo el tema de Comunicaciones.                */
/***************************************************************************/
// Array de direcciones para usuarios conectados
extern LocalNodeAddress IPX_Users[MAX_USERS];
extern LocalNodeAddress IPX_MyAddress;
extern WORD             whoami;
extern WORD             Ack;
extern WORD             GlobalTimeOut; // 1 Time Out

/***************************************************************************/
/* Rutinas para facilitar todo el tema de Comunicaciones.                  */
/***************************************************************************/

VOID InitECB(ECB far *MyECB,WORD WhatPack);
VOID InitECBs();
WORD InsertIfNot(LocalNodeAddress *NewAddress);
WORD InsertIn(LocalNodeAddress *NewAddress, int posicion);
WORD SendPacket(WORD Player);
WORD SendPacketToPlayer(WORD Player,WORD Command,char *Buffer,WORD Lengt);
VOID SendPacketToAll(WORD Command,char *Buffer,WORD Lengt);
WORD CheckPackets();
WORD InitIPX(WORD socket);
VOID EndIPX();
WORD CheckLookPackets();
VOID InitWaitForOtherUsers();
WORD LoopWaitForOtherUsers(WORD Continue,WORD WaitFor);
VOID EndWaitForOtherUsers();
//DWORD GetInternalPacket(WORD Usuario,WORD Comando,BYTE *Buffer,WORD Len);
DWORD GetInternalPacket(WORD OLDESTECB,LocalNodeAddress *IPX_WorkAddress);
VOID SetLightServer(WORD *DirUsers, WORD number);
VOID KillLightServer();
BYTE *ShowInternalAddress(LocalNodeAddress far *NetWorkNode);
void ClearECBS();
void ClearPaquete();
int looking_for_games();
int join_game(int num_game);
int ipx_exit_game();

#ifdef __cplusplus
}
#endif
#endif
