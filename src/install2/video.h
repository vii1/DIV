#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "main.h"
#include "vesa.h"
#include "rect.h"

#define PALETTE_SIZE ( 256 * 3 )

int	 video_set_mode();
void video_reset();
void retrazo();
void set_dac( const byte* _dac );
void fundido( int alfa );
void fade_on();
void fade_off();
void pal_init();
void volcado();
void volcado_parcial( Rect r );

void put_screen( const byte* map );
void put( const byte* map, Rect rect );
void put_raw( const byte* map, Rect rect );
void get( byte* dst, Rect rect );

extern byte* paleta;
extern VBESCREEN screen;

#endif	 // __VIDEO_H__
