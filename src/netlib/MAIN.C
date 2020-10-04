#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <i86.h>
#include "net.h"

char errores[5][32]={"NET_INITOK","NET_ABORTED","NET_BADINIT","NET_TIMEOUT","NET_BADSYNC"};

//=============================================================================
// Programa de ejemplo
//=============================================================================

void main(int argc,char *argv[])
{
  int quien_soy=atoi(argv[1]);
  int exit_program=0;
  int game_id=3040622;
  char prueba[16][64];
  int contador=0;
  int player_id;

  net.dispositivo=IPX;
  net.num_players=8;

/*
  if (_net_get_games(game_id)==0) { // No hay partidas activas
    net_end();
    return;
  }
*/
  player_id=_net_join_game(game_id,"MiPartida",prueba,64);

  if (player_id<0) {
    net_end();
    return;
  }

  while(!exit_program) {
    sprintf(prueba[player_id],"Prueba del jugador %d -%d-",player_id,contador);
    contador++;
    net_loop();
//    printf("Paquete recibido: Msg=\"%s\"\n",prueba[3-player_id]);

    if (kbhit()) if (getch()==27) break;
  }

  net_end();
}

