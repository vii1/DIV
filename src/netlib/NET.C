//=============================================================================
//  Modulo central de la libreria de red
//  Programado por: Luis F. Fernandez
//=============================================================================

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <i86.h>
#include <time.h>

#include "net.h"
#include "..\source\inter.h"

#define NET_TIMEOUT 1
#define NET_NOTINIT -1

char net_games[10][32];
int net_players[16];

int inicializacion_red=0;

net_game partida_red;

extern int reloj;

//=============================================================================
//  Procedimientos internos
//=============================================================================

int net_init(int game_id);
void net_end();
void net_exit_game();
int net_init_ipx(int game_id);
int net_init_serial(int game_id);
void net_send();
int net_create_game(int game_id,char *nombre, int num_jugadores);
int old_net_reloj;

//=============================================================================
// Inicializa la comunicaci¢n
//=============================================================================

int net_init(int game_id)
{
  old_net_reloj=reloj;
  switch (net->dispositivo)
  {
    case IPX:     return(net_init_ipx(game_id));
                  break;
    case SERIAL:  return(net_init_serial(game_id));
                  break;
    case MODEM:   return(net_init_modem(5));
                  break;
  }
  e(162);
  return(-1);
}

//=============================================================================
// Inicializa la comunicaci¢n a traves de ipx
//=============================================================================

int net_init_ipx(int game_id)
{
  int socket=27350;

  //---------------------------------------------------------------------------
  // Compruebo si se inici¢ la comunicacion, en caso afirmativo se finaliza
  //---------------------------------------------------------------------------
  if (inicializacion_red)
    net_end();

  //---------------------------------------------------------------------------
  // Inicializa el socket
  //---------------------------------------------------------------------------
  if( !InitIPX(socket) )
  {
    return( NET_BADINIT );
  }

  //---------------------------------------------------------------------------
  // Selecciona el dispositivo
  //---------------------------------------------------------------------------
  InitECBs();
  partida_red.dispositivo=IPX;

  //---------------------------------------------------------------------------
  // Inicializaci¢n correcta
  //---------------------------------------------------------------------------
  inicializacion_red=1;
  partida_red.game_id=game_id;
  partida_red.activa=0;
  partida_red.servidor=0;
  partida_red.player_id=0;
  return(0);
}

//=============================================================================
// Inicializa la comunicaci¢n a traves de cable serie
//=============================================================================

int net_init_serial(int game_id)
{
  //---------------------------------------------------------------------------
  // Compruebo si se inici¢ la comunicacion, en caso afirmativo se finaliza
  //---------------------------------------------------------------------------
  if (inicializacion_red)
    net_end();

  partida_red.num_players=2;

  //---------------------------------------------------------------------------
  // Inicializa el puerto
  //---------------------------------------------------------------------------
  if (InitCOM(net->com,net->velocidad)) {
    return(NET_BADINIT);
  }

  //---------------------------------------------------------------------------
  // Selecciona el dispositivo
  //---------------------------------------------------------------------------
  partida_red.dispositivo=SERIAL;

  //---------------------------------------------------------------------------
  // Inicializaci¢n correcta
  //---------------------------------------------------------------------------
  inicializacion_red=1;
  partida_red.game_id=game_id;
  partida_red.activa=0;
  partida_red.servidor=0;
  COM_Send(1,MSG_SYNC,"SYNC",4);
  return(0);
}

//=============================================================================
// Inicializa la comunicaci¢n a traves de modem
//=============================================================================

int net_init_modem(int timeout)
{
char data_aux[64];
int dial=0;//????????? FERNANDO ?????
/*
  if ( net->telefono[0]==NULL )     // OJO //OKFER
    dial=0;
  else
    dial=1;
*/
  if (net->tonos)
    strcpy(data_aux,"ATDT");
  else
    strcpy(data_aux,"ATDP");

//  strcat(data_aux,net->telefono);

  //---------------------------------------------------------------------------
  // Compruebo si se inici¢ la comunicacion, en caso afirmativo se finaliza
  //---------------------------------------------------------------------------
  if (inicializacion_red)
    net_end();

  partida_red.num_players=2;
  timeout*=CLOCKS_PER_SEC;

  //---------------------------------------------------------------------------
  // Inicializa el puerto
  //---------------------------------------------------------------------------
  if (InitCOM(net->com,net->velocidad)) {
    return(NET_BADINIT);
  }

  //---------------------------------------------------------------------------
  // Inicializa el modem
  //---------------------------------------------------------------------------
  partida_red.servidor=dial;  // El que marca es uno y el que recibe la llamada 0.
                  // (El servidor espera, el nodo llama)
/*
//  InitModem("ATZ", "ATDT5197575");
  if (InitModem(net->cadena_inicio,data_aux)) {
    return(NET_BADINIT);
  }
*/
  //---------------------------------------------------------------------------
  // Selecciona el dispositivo
  //---------------------------------------------------------------------------
  partida_red.dispositivo=MODEM;

  //---------------------------------------------------------------------------
  // Inicializaci¢n correcta
  //---------------------------------------------------------------------------
  inicializacion_red=1;
  return(partida_red.servidor);
}

//=============================================================================
// Control de paquetes (para servidores)
//=============================================================================

void _net_loop()
{
  static con_reloj=0;

  //---------------------------------------------------------------------------
  // Compruebo si se inici¢ la comunicacion, en caso contrario retorna
  //---------------------------------------------------------------------------
  if (!inicializacion_red)
    return;

  //---------------------------------------------------------------------------
  // Compruebo si hay una partida activa, en caso contrario retorna
  //---------------------------------------------------------------------------
  if (!partida_red.activa)
    return;

  //---------------------------------------------------------------------------
  // Manda un paquete
  //---------------------------------------------------------------------------
  net->servidor=partida_red.servidor;
  net->tonos=partida_red.game_id;
  con_reloj+=(reloj-old_net_reloj);
  old_net_reloj=reloj;
  if(con_reloj>2)
  {
    con_reloj-=2;
    net_send();
  }

  //---------------------------------------------------------------------------
  // Chequea los paquetes en la red
  //---------------------------------------------------------------------------
  if (partida_red.dispositivo==IPX)
  {
    do {
    } while (CheckPackets());
//    CheckPackets();
  }
  else
  {
    COM_CheckPacket();
  }
  net->act_players=partida_red.num_players;
  //---------------------------------------------------------------------------
  // Si la partida ya est  completa retorna
  //---------------------------------------------------------------------------
  if (partida_red.num_players==partida_red.max_players) {
    return;
  }
}

//=============================================================================
//  Finaliza la red
//=============================================================================

void net_end()
{
  if (!inicializacion_red)
    return;

  net_exit_game();

  if (partida_red.dispositivo==IPX)
    EndIPX();
  else
    EndCOM();

  inicializacion_red=0;
}

//=============================================================================
// Manda un paquete
//=============================================================================

void net_send()
{
/*
  int i;

  if (partida_red.servidor)
    printf("Jugadores: %d, NumPlayer: %d, Dir: %s, Soy el Servidor\n",partida_red.num_players,partida_red.player_id,ShowInternalAddress(&IPX_MyAddress));
  else
    printf("Jugadores: %d, NumPlayer: %d, Dir: %s, Soy un Nodo\n",partida_red.num_players,partida_red.player_id,ShowInternalAddress(&IPX_MyAddress));
*/
  if (!partida_red.activa || (partida_red.num_players<2 && partida_red.dispositivo==IPX))
    return;

  if (partida_red.dispositivo==IPX) {
    SendPacketToAll(partida_red.player_id,(void *)((int)partida_red.datos+partida_red.player_id*partida_red.longitud_datos),partida_red.longitud_datos);
  }
  else {
    COM_Send(partida_red.servidor,0,partida_red.datos,partida_red.longitud_datos);
  }
}

//=============================================================================
// Crea una partida
//=============================================================================

int net_create_game(int game_id,char *nombre, int num_jugadores)
{
  //---------------------------------------------------------------------------
  // Compruebo si se inici¢ la comunicacion, en caso contrario retorna
  //---------------------------------------------------------------------------
  if (!inicializacion_red)
    net_init(game_id);

  //---------------------------------------------------------------------------
  // Compruebo si hay una partida activa, en caso afirmativo retorna
  //---------------------------------------------------------------------------
  if (partida_red.activa)
    return(-1);

  //---------------------------------------------------------------------------
  // Compruebo si el n£mero de jugadores es correcto, en caso contrario retorna
  //---------------------------------------------------------------------------
  if (partida_red.dispositivo==IPX) {
    if (num_jugadores<0 || num_jugadores>=MAX_USERS)
      return(NET_BADNUMPLAYERS);
  }
  else {
    if (num_jugadores!=2)
      return(NET_BADNUMPLAYERS);
  }

  strcpy(partida_red.nombre,nombre);
  partida_red.activa=1;
  partida_red.servidor=1;
  net->servidor=1;
  partida_red.max_players=num_jugadores;

  if (partida_red.dispositivo==IPX)
    InsertIn(&IPX_MyAddress,0);

  //---------------------------------------------------------------------------
  // Inicializaci¢n correcta
  //---------------------------------------------------------------------------
  return(0);
}

//=============================================================================
// Destruye una partida
//=============================================================================

void net_exit_game()
{
  //---------------------------------------------------------------------------
  // Compruebo si se inici¢ la comunicacion, en caso contrario retorna
  //---------------------------------------------------------------------------
  if (!inicializacion_red)
    return;
  if (!partida_red.activa)
    return;

  partida_red.activa=0;

  //---------------------------------------------------------------------------
  // Enviar a los otros usuarios el mensaje de salida
  //---------------------------------------------------------------------------
  if (partida_red.dispositivo==IPX) {
    ipx_exit_game();
  }
  else {
    if (partida_red.servidor)
      COM_Send(1,MSG_EXIT,"",0);
    else
      COM_Send(0,MSG_EXIT,"",0);
  }

}

//=============================================================================
// Busca las partidas activas
//=============================================================================

int _net_get_games(int game_id)
{
  int num_games;
  int valor_retorno;

  //---------------------------------------------------------------------------
  // Compruebo si se inicio la comunicacion, en caso contrario la inicia
  //---------------------------------------------------------------------------
  if (!inicializacion_red) {
    if ((valor_retorno=net_init(game_id))<0)
      return(valor_retorno);
  }

  if (partida_red.dispositivo==IPX) {
    num_games=looking_for_games(1);
    return(num_games);
  }
  return(1);
}

//=============================================================================
// Entra en una de las partidas activas
//=============================================================================

int _net_join_game(int game_id,char *nombre,void *datos, int longitud)
{
int num_games,i;
int valor_retorno;

  //---------------------------------------------------------------------------
  // Comprueba el numero de jugadores
  //---------------------------------------------------------------------------
  if (net->num_players<=0 || net->num_players>16)
  {
    e(149);
    return(-5);
  }

  //---------------------------------------------------------------------------
  // Inicializa la tabla de jugadores
  //---------------------------------------------------------------------------
  for (i=0;i<net->num_players;i++) {
    net_players[i]=0;
  }
  partida_red.num_players=0;

  //---------------------------------------------------------------------------
  // Comprueba que le paquete no supere los 200 bytes
  //---------------------------------------------------------------------------
// FER_AQUI
  longitud*=4;
//  longitud/=net->num_players;  
  if (longitud>=200)
  {
    e(163);
    return(-6); // Paquete demasiado grande
  }

  //---------------------------------------------------------------------------
  // Compruebo si se inici¢ la comunicacion, en caso contrario retorna
  //---------------------------------------------------------------------------
  if (!inicializacion_red) {
    if ((valor_retorno=net_init(game_id))<0)
      return(valor_retorno);
  }
  if (partida_red.activa) {
    net_exit_game();
    return(0);
  }

  partida_red.datos=datos;
  partida_red.longitud_datos=longitud;

  //---------------------------------------------------------------------------
  // Busca si existe la partida
  //---------------------------------------------------------------------------
  if (num_games=_net_get_games(game_id)) {
    for (i=0;i<num_games;i++) {
      if (!stricmp(net_games[i],nombre)) {
        if (partida_red.dispositivo==IPX)
          return(join_game(num_games));
        else
          return 1;
      }
    }
  }
  return(net_create_game(game_id,nombre,net->num_players));
}

