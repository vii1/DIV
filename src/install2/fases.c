#include "video.h"
#include "fpg.h"
#include "mouse.h"
#include "keyboard.h"
#include "time.h"
#include "rect.h"
#include "fases.h"

static byte* fondoMouse;
static Rect	 mouseRect, prevMouseRect;
static int	 mousecx, mousecy;
static int	 mouseRedibuja = 0;

static void mouseInit()
{
	fpg_map_center( FPG_MOUSE, &mousecx, &mousecy );
	mouseRect = rect( mouseX - mousecx, mouseY - mousecy, fpgIndex[FPG_MOUSE]->width, fpgIndex[FPG_MOUSE]->height );
	prevMouseRect = mouseRect;
	fondoMouse = div_malloc( mouseRect.an * mouseRect.al );
	get( fondoMouse, mouseRect );
	put( fpg_map( FPG_MOUSE ), mouseRect );
}

static void mouseBorra()
{
	read_mouse();
	if( mouseX != prevMouseX || mouseY != prevMouseY ) {
		prevMouseRect = mouseRect;
		mouseRect.x = mouseX - mousecx;
		mouseRect.y = mouseY - mousecy;
		put_raw( fondoMouse, prevMouseRect );
	}
}

static void mouseDibuja()
{
	if( mouseRedibuja || mouseX != prevMouseX || mouseY != prevMouseY ) {
		get( fondoMouse, mouseRect );
		put( fpg_map( FPG_MOUSE ), mouseRect );
		volcado_parcial( prevMouseRect );
		volcado_parcial( mouseRect );
		mouseRedibuja = 0;
	}
}

static void mouseCheck( Rect r )
{
	if( rectRect( r, mouseRect ) ) mouseRedibuja = 1;
}

static void lee_teclado()
{
	static float vx = 0, vy = 0;
	float		 vmax = 14.0f / time10frames - (float)mouseSpeed / 15.0f;
	int			 keymouse = 0;
	tecla();
	if( key( _LEFT ) ) {
		vx -= vmax;
		keymouse |= 1;
	}
	if( key( _RIGHT ) ) {
		vx += vmax;
		keymouse |= 1;
	}
	if( key( _UP ) ) {
		vy -= vmax;
		keymouse |= 2;
	}
	if( key( _DOWN ) ) {
		vy += vmax;
		keymouse |= 2;
	}
	if( vx || vy ) {
		scan_code = 0;
		if( vx < -vmax * 10.0 ) vx = -vmax * 10.0;
		if( vx > vmax * 10.0 ) vx = vmax * 10.0;
		if( vy < -vmax * 10.0 ) vy = -vmax * 10.0;
		if( vy > vmax * 10.0 ) vy = vmax * 10.0;
		moveMouse( vx, vy );
	}
	if( !( keymouse & 1 ) ) {
		if( vx > 1.0 )
			vx -= 2.0;
		else if( vx < -1.0 )
			vx += 2.0;
		else
			vx = 0.0;
	}

	if( !( keymouse & 2 ) ) {
		if( vy > 1.0 )
			vy -= 2.0;
		else if( vy < -1.0 )
			vy += 2.0;
		else
			vy = 0.0;
	}
}

Rect calcula_rect( int code, int x, int y )
{
	Rect rect;
	int	 cx, cy;
	fpg_map_center( code, &cx, &cy );
	rect.x = x - cx;
	rect.y = y - cy;
	rect.an = fpgIndex[code]->width;
	rect.al = fpgIndex[code]->height;
	return rect;
}

void dibuja_centrado( int code, int x, int y )
{
	Rect rect = calcula_rect( code, x, y );
	put( fpg_map( code ), rect );
	mouseCheck( rect );
}

void fasePrincipal()
{
	int	  salir = 0;
	int	  unidad = 0;
	Rect  rectUnidad[4];
	int	  i;

	put_screen( fpg_map( FPG_FONDO ) );

	for( i = 0; i < 4; ++i ) {
		int	 px = fpgIndex[FPG_FONDO]->cpoints[8 + i].x;
		int	 py = fpgIndex[FPG_FONDO]->cpoints[8 + i].y;
		int	 code = i == 0 ? 10 : 11;
		Rect r = calcula_rect( code, px, py );
		rectUnidad[i] = r;
		put( fpg_map( code ), r );
	}

	mouseRedibuja = 1;

	mouseDibuja();
	volcado();
	fade_on();
	do {
		lee_teclado();
		mouseBorra();
		if( key( _ESC ) ) salir = 1;
		mouseDibuja();
		retrazo();
	} while( !salir );
	fade_off();
}

void faseIntro()
{
	// Imagen intro
	put_screen( fpg_map( FPG_INTRO ) );
	mouseInit();
	volcado();
	fade_on();
	do {
		lee_teclado();
		mouseBorra();
		mouseDibuja();
		retrazo();
	} while( !mouseButtons && !ascii );
	fade_off();
	fasePrincipal();
}
