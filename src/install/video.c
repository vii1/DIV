#include <string.h>
#include <i86.h>
#include <conio.h>
#include "vesa.h"
#include "graph.h"
#include "video.h"

VBESCREEN screen;
byte	  paleta_activa[PALETTE_SIZE];
byte	  c0;
byte*	  buffer;
byte*	  paleta;

int video_set_mode()
{
	if( vbeInit() != 0 ) {
		return 1;
	}
	if( vbeSetMode( 640, 480, 8, &screen ) != 0 ) {
		return 1;
	}
	buffer = div_malloc( 640 * 480 );
	return 0;
}

void video_reset()
{
	_setvideomode( _TEXTC80 );
}

void retrazo()
{
	while( inp( 0x3da ) & 8 ) {
	}
	while( ( inp( 0x3da ) & 8 ) == 0 ) {
	}
}

void set_dac( const byte* _dac )
{
	union REGS regs;
	int		   n = 0;

	retrazo();

	outp( 0x3c8, 0 );
	do {
		outp( 0x3c9, _dac[n++] );
	} while( n < 768 );

	// Color del borde
	regs.w.ax = 0x1001;
	regs.h.bh = c0;
	int386( 0x010, &regs, &regs );
}

/*
 * alfa = 0 -> negro
 * alfa = 63 -> color original
 */
void fundido( const byte* paleta, int alfa )
{
	if( alfa <= 0 ) {
		memset( paleta_activa, 0, PALETTE_SIZE );
	} else if( alfa >= 63 ) {
		memcpy( paleta_activa, paleta, PALETTE_SIZE );
	} else {
		int i;
		for( i = 0; i < PALETTE_SIZE; ++i ) {
			paleta_activa[i] = paleta[i] * alfa / 63;
		}
	}
	set_dac( paleta_activa );
}

void fade_on()
{
	int i;
	for( i = 0; i <= 63; ++i ) {
		fundido( paleta, i );
		retrazo();
	}
}

void fade_off()
{
	int i;
	for( i = 63; i >= 0; --i ) {
		fundido( paleta, i );
		retrazo();
	}
}

byte find_color( const byte* pal, byte r, byte g, byte b )
{
	int			dmin, r2, g2, b2;
	const byte *endpal, *color;

	color = pal;
	endpal = pal + PALETTE_SIZE;
	dmin = 64 * 64;
	r2 = (int)r * r;
	g2 = (int)g * g;
	b2 = (int)b * b;
	do {
		int d = (int)*pal - r2;
		int dif = d * d;
		pal++;
		d = (int)*pal - g2;
		dif += d * d;
		pal++;
		d = (int)*pal - b2;
		dif += d * d;
		pal++;
		if( dif < dmin ) {
			dmin = dif;
			color = pal - 3;
		}
	} while( pal < endpal );
	return ( color - pal ) / 3;
}

void pal_init()
{
	c0 = find_color( paleta, 0, 0, 0 );
}

void volcado( const byte* src )
{
	memcpy( screen.adr, src, screen.xres * screen.yres );
}