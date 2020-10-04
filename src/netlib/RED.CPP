#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

#include "net.h"
#include "..\source\inter.h"

int _net_get_games(int game_id);
int _net_join_game(int game_id,char *nombre,void *datos, int longitud);

#define text_offset mem[7] // Start of text segment (mem[] index)
int                 net_game_id;

//-----------------------------------------------------------------------------
//  Inicializacion del identificador de juego
//-----------------------------------------------------------------------------

void net_create_game_id()
{
  int i;
  net_game_id=0;

  for (i=0;i<9;i++) {
    net_game_id+=mem[i];
  }
  net_game_id=abs(net_game_id);
  if (net_game_id<10500)
    net_game_id+=10500;

}

//-----------------------------------------------------------------------------
//  Entra en una de las partidas activas
//-----------------------------------------------------------------------------

void net_join_game()
{
int game_id;
int c;
int longitud=pila[sp--];
int datos   =pila[sp--];
int nombre  =pila[sp];

  game_id=net_game_id+nombre+mem[text_offset+nombre];
  c=_net_join_game( game_id, (char *)&mem[text_offset+nombre], (void *)&mem[datos], longitud );
  pila[sp]=c;
}

//-----------------------------------------------------------------------------
//  Entra en una de las partidas activas
//-----------------------------------------------------------------------------

void net_get_games()
{
int c;

  c=_net_get_games(net_game_id);
  pila[++sp]=c;
}

