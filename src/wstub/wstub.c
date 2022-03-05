
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include <malloc.h>

#include "../wstub.h"

#define MAX_CMDLINE 128

void get_cpuid( void );

// 0=8086 processor
// 2=Intel 286 processor
// 3=Intel386(TM) processor
// 4=Intel486(TM) processor
// 5=Pentium(TM) processor
extern char cpu_type;

void set_mode( void );

// TODO: sacar esto a un asm o buscar una alternativa en C
// clang-format off
#pragma aux set_mode= \
    "push ax"         \
    "mov ax,3"        \
    "int 10h"         \
    "pop ax"          \
    parm[] value[ax]
// clang-format on

//
// Devuelve true si DIV est  ya en ejecuci¢n
//
int is_div_running()
{
	// Usamos el prompt para saber si estamos intentando ejecutar DIV desde un shell dentro de DIV
	// TODO: Buscar otra forma menos chapucera de hacer esto
	char* p = getenv( "PROMPT" );
	return p && p[0] == '[' && p[1] == 'D' && p[2] == 'I' && p[3] == 'V' && p[4] == ']';
}

//
// Cambia el directorio actual al directorio de DIV
//
void chdir_to_div( const char* argv0 )
{
	char	 full[_MAX_PATH + 1];
	unsigned n;

	_fullpath( full, argv0, _MAX_PATH + 1 );
	// strupr( full );
	n = strlen( full );
	while( n && full[n] != '\\' ) n--;
	full[n] = 0;
	if( full[n - 1] == ':' ) strcat( full, "\\" );
	_dos_setdrive( (int)toupper( full[0] ) - 'A' + 1, &n );
	_chdir( full );
}

//
// Ejecuta un proceso con los par metros dados
//
int spawn_process( const char* const* argv )
{
	flushall();
	_heapmin();
	_heapshrink();
	return spawnvp( P_WAIT, argv[0], argv );
}

//
// Ejecuta el juego compilado (F10)
//
int exec_game()
{
	const char* args[4];
	unsigned	drive, total;
	int			retval;

	_dos_getdrive( &drive );
	args[0] = "SYSTEM\\DOS4GW.EXE";
	if( cpu_type >= 5 ) {
		args[1] = "SYSTEM\\SESSION.DIV";
	} else {
		args[1] = "SYSTEM\\SESSION.386";
	}
	args[2] = "SYSTEM\\EXEC.EXE";
	args[3] = NULL;
	retval = spawn_process( args );
	set_mode();
	_dos_setdrive( drive, &total );
	return retval;
}

//
// Main
//
int main( int argc, char* argv[] )
{
	const char* args[5];
	char		cmdline[MAX_CMDLINE];
	int			retval = 1;

	get_cpuid();
	if( cpu_type < 3 ) {
		printf( "Error: 80386 or greater processor not found!" );
		return 1;
	}

	if( is_div_running() ) {
		printf( "DIV Games Studio is already loaded, type EXIT to enter.\n" );
		return 1;
	}

	chdir_to_div( argv[0] );

	args[0] = "SYSTEM\\DOS4GW.EXE";
	if( cpu_type >= 5 ) {
		args[1] = argv[0];
	} else {
		args[1] = "SYSTEM\\D.386";
	}
	args[2] = "INIT";
	_bgetcmd( cmdline, MAX_CMDLINE );
	args[3] = cmdline;
	args[4] = NULL;

	putenv( "DOS4G=QUIET" );

	// La siguiente l¡nea daba problemas de arranque en algunos equipos ...
	// putenv("DOS4GVM=VIRTUALSIZE#65536 MINMEM#2048 MAXMEM#65536");

	while( retval > 0 && retval < 128 ) {
		retval = spawn_process( args );
		cmdline[0] = 0;
		if( retval == -1 ) {
			puts( "Stub exec failed:" );
			puts( args[0] );
			puts( strerror( errno ) );
			return 1;
		}
		if( retval == DIV_RETVAL_EXEC_GAME ) {
			if( exec_game() == -1 ) {
				args[2] = "ERROR";
			} else {
				args[2] = "NEXT";
			}
		} else if( retval == DIV_RETVAL_TEST_MODE ) {
			args[2] = "TEST";
		} else {
			args[2] = "NEXT";
		}
	}

	if( retval == DIV_RETVAL_ENOMEM ) {
		printf( "\n[DIV] Critical error: Not enough memory!\n" );
	} else if( retval == DIV_RETVAL_EINT ) {
		printf( "\n[DIV] Critical error: User interrupt!\n" );
	} else if( retval == 0 ) {
		return 0;
	}
	return retval;
}
