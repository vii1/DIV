#include <string.h>
#include <i86.h>
#include <conio.h>
#include "vesa.h"
#include "graph.h"
#include "time.h"
#include "video.h"

#define FADE_TIME ( TICKS_PER_SEC / 4 )

VBESCREEN screen;
byte	  paleta_activa[PALETTE_SIZE];
byte	  c0;
byte*	  buffer;
byte*	  paleta;
word	  scanline;

int video_set_mode()
{
	if( vbeInit() != 0 ) {
		return 1;
	}
	if( vbeSetMode( 640, 480, 8, &screen ) != 0 ) {
		return 1;
	}
	scanline = ModeInfoBlock.BytesPerScanLine;
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
void fundido( int alfa )
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
	for( i = 0; i < FADE_TIME * 10; i += time10frames ) {
		fundido( i * 63 / (FADE_TIME * 10) );
		retrazo();
	}
	fundido( 63 );
}

void fade_off()
{
	int i;
	for( i = FADE_TIME * 10; i >= 0; i -= time10frames ) {
		fundido( i * 63 / (FADE_TIME * 10) );
		retrazo();
	}
	fundido( 0 );
}

static byte find_color( const byte* pal, byte r, byte g, byte b )
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

void volcado()
{
	if( scanline == screen.xres ) {
		memcpy( screen.adr, buffer, screen.xres * screen.yres );
	} else {
		const byte* src = buffer;
		byte*		p = screen.adr;
		int			i = 0;
		for( ; i < screen.yres; ++i, p += scanline, src += screen.xres ) {
			memcpy( p, src, screen.xres );
		}
	}
}

void recortar( Rect* r )
{
	if( r->x < 0 ) {
		r->an += r->x;
		r->x = 0;
	}
	if( r->y < 0 ) {
		r->al += r->y;
		r->y = 0;
	}
	if( r->x + r->an - 1 >= screen.xres ) {
		r->an = screen.xres - r->x;
	}
	if( r->y + r->al - 1 >= screen.yres ) {
		r->al = screen.yres - r->y;
	}
	if( r->an < 0 ) r->an = 0;
	if( r->al < 0 ) r->al = 0;
}

void volcado_parcial( Rect r )
{
	byte *dst, *src;
	recortar( &r );
	if( r.an == 0 || r.al == 0 ) return;
	dst = screen.adr + scanline * r.y + r.x;
	src = buffer + screen.xres * r.y + r.x;
	for( ; r.al > 0; --r.al, dst += scanline, src += screen.xres ) {
		memcpy( dst, src, r.an );
	}
}

void put_screen( const byte* map )
{
	memcpy( buffer, map, screen.xres * screen.yres );
}

void put( const byte* map, Rect rect )
{
	byte* dst;
	Rect  t = rect;
	recortar( &t );
	if( t.an == 0 || t.al == 0 ) return;
	dst = buffer + screen.xres * t.y + t.x;
	map += ( t.y - rect.y ) * rect.an + ( t.x - rect.x );
	for( ; t.al > 0; --t.al, dst += screen.xres, map += rect.an ) {
		register int x = t.an - 1;
		for( ; x >= 0; --x ) {
			register byte b = map[x];
			if( b ) dst[x] = b;
		}
	}
}

void put_raw( const byte* map, Rect rect )
{
	byte* dst;
	Rect  t = rect;
	recortar( &t );
	if( t.an == 0 || t.al == 0 ) return;
	dst = buffer + screen.xres * t.y + t.x;
	map += ( t.y - rect.y ) * rect.an + ( t.x - rect.x );
	for( ; t.al > 0; --t.al, dst += screen.xres, map += rect.an ) {
		memcpy( dst, map, t.an );
	}
}

void get( byte* dst, Rect r )
{
	byte* src;
	Rect  t = r;
	recortar( &t );
	if( t.an <= 0 || t.al <= 0 ) return;
	src = buffer + t.y * screen.xres + t.x;
	dst += r.an * ( t.y - r.y ) + ( t.x - r.x );
	for( ; t.al > 0; --t.al, dst += r.an, src += screen.xres ) {
		memcpy( dst, src, t.an );
	}
}

Rect rect( int x, int y, int an, int al )
{
	Rect r;
	r.x = x;
	r.y = y;
	r.an = an;
	r.al = al;
	return r;
}
