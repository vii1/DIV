/*
 * Timer handler, calls a user function at the desired PIT clock frequency.
 *
 * Doesn't "officially" belong to JUDAS but feel free to use! This is
 * blasphemy-ware too!
 */

#include <conio.h>
#include <dos.h>
#include "video.h"
#include "time.h"

unsigned int reloj;
unsigned int time10frames;

void __interrupt timer_handler( void );
unsigned short	 timer_get_ds( void );

#pragma aux timer_get_ds = "mov ax, ds" modify[ax] value[ax];

static unsigned char timer_initialized = 0;
extern void( __interrupt __far* timer_oldvect )();
extern void( __interrupt __far* timer_newvect )() = &timer_handler;
extern void ( *timer_function )();
extern unsigned		  timer_count;
extern unsigned short timer_frequency;
extern unsigned short timer_systemcount;
extern unsigned short timer_ds;
extern int			  timer_code_lock_start;
extern int			  timer_code_lock_end;

int timer_init1( unsigned short frequency, void ( *function )() )
{
	if( timer_initialized ) return 1;
	timer_function = function;
	timer_count = 0;
	timer_systemcount = 0;
	timer_frequency = frequency;
	timer_ds = timer_get_ds();
	timer_oldvect = _dos_getvect( 8 );
	_disable();
	_dos_setvect( 8, timer_newvect );
	outp( 0x43, 0x34 );
	outp( 0x40, frequency );
	outp( 0x40, frequency >> 8 );
	_enable();
	timer_initialized = 1;
	return 1;
}

static void func_100Hz()
{
	++reloj;
}

int timer_init()
{
	reloj = 0;
	return timer_init1( 1193180 / TICKS_PER_SEC, func_100Hz );
}

void timer_uninit( void )
{
	if( !timer_initialized ) return;
	_disable();
	_dos_setvect( 8, timer_oldvect );
	outp( 0x43, 0x34 );
	outp( 0x40, 0x00 );
	outp( 0x40, 0x00 );
	_enable();
	timer_initialized = 0;
}

void measure_time()
{
	// Medimos el tiempo de 10 frames
	int			 i;
	unsigned int inicio = reloj;
	for( i = 0; i < 10; ++i ) {
		retrazo();
	}
	time10frames = reloj - inicio;
}
