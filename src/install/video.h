#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "main.h"

#define PALETTE_SIZE ( 256 * 3 )

int	 video_set_mode();
void video_reset();
void retrazo();
void set_dac( const byte* _dac );
// void fundido( const byte* paleta, int alfa );
void fade_on();
void fade_off();
void pal_init();
void volcado( const byte* src );

extern byte* paleta;

#endif	 // __VIDEO_H__
