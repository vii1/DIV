#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <libgen.h>
#include <direct.h>

// #define DEFINIR_AQUI
// #include "global.h"
// #undef DEFINIR_AQUI

typedef unsigned char  byte;
typedef unsigned short word;

#define max_windows 96	  // N§ m ximo de ventanas abiertas
#define max_textos 2048	  // N§ m x. de textos permitidos (lenguaje.div)

byte* texto[max_textos];   // Textos de salida, en formato traducible

#define max_items 24   // N§ m ximo de objetos en una ventana

struct t_item {
	int tipo;	  // 0-ninguno,1-boton,2-get,3-switch
	int estado;	  // Estado del item (raton sobre ‚l, pulsado o no ...)
	union {
		struct {
			byte* texto;
			int	  x, y, center;
		} button;
		struct {
			byte* texto;
			byte* buffer;
			int	  x, y, an, lon_buffer;
			int	  r0, r1;
		} get;
		struct {
			byte* texto;
			int*  valor;
			int	  x, y;
		} flag;
	};
};

struct tventana {
	int tipo;					  // 0-none, 1-dialogo, 2-menu, 3-paleta
								  // 4-timer, 5-papelera, 7-barra_progreso
								  // 8-mixer
								  // 100-map (>=100 objetos excluibles)
								  // 101-fpg 102-prg-gen_fnt-hlp 104-fnt
								  // 105-pcm 106-map3D 107-modulo de musica
	int			  orden;		  // N§ de orden de la ventana
	int			  primer_plano;	  // 0-No, 1-Si, 2-Minimizada
	byte*		  nombre;		  // Nombre del icono representativo
	byte*		  titulo;		  // Nombre en la barra de t¡tulo
	int			  paint_handler, click_handler, close_handler;
	int			  x, y, an, al;		  // Posici¢n y dimensiones de la ventana
	int			  _x, _y, _an, _al;	  // Posici¢n salvada al minimizarse
	byte*		  ptr;				  // Buffer de la ventana
	struct tmapa* mapa;				  // Puntero a otro struct tipo mapa
	struct tprg*  prg;				  // Puntero a otro struct tipo prg
	int			  volcar;			  // Indica si se debe volcar la ventana
	int			  estado;			  // Estado de selecciones, botones, ...
	int			  botones;			  // Estado de botones (pulsados o no)
	byte*		  aux;				  // Puntero auxiliar para chorradas varias
	struct t_item item[max_items];	  // Botones, gets, switches, etc...
	int			  items;			  // N§ de objetos definidos
	int			  active_item;		  // Cuando alg£n item produce un efecto
	int			  selected_item;	  // El item seleccionado (para teclado)
	int			  lado;				  // 0 Derecha, 1 Izquierda (autoemplazar con doble click)
};

struct tventana ventana[max_windows];
#define v ventana[0]

int	  big2 = 1;
byte  c2 = 0, c3 = 0, c4 = 0;
FILE* f = NULL;
char  full[_MAX_PATH + 1];
void* Setupfile;
int	  v_ventana;
int	  mouse_b, mouse_graf;
int	  fin_dialogo;
int	  v_ayuda;
char  ascii, scan_code;

void inicializa_textos( byte* fichero );
void finaliza_textos( void );

typedef struct {
	int max_process;
	int ignore_errors;
	int free_sintax;
	int extended_conditions;
	int simple_conditions;
	int case_sensitive;
	int comprueba_rango;
	int comprueba_id;
	int comprueba_null;
	int hacer_strfix;
	int optimizar;
} t_compiler_options;

char* original_cwd;
int	  verbose = 0;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Compilaci¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

// copiado de div.cpp
byte lower[256] = "                                   #$           0123456789      "
				  " abcdefghijklmnopqrstuvwxyz    _ abcdefghijklmnopqrstuvwxyz     "
				  "‡ueaaaa‡eeeiiiaae‘‘ooouuyou›œžŸaiou¤¤¦§                        "
				  "                                                                ";

byte* source_ptr;
int	  source_len;
int	  saved_esp;

void inicializa_compilador( void );
void compilar_programa( void );
void finaliza_compilador( void );

void compilar( void );	 // Funciones internas del compilador
void comp( void );
void free_resources( void );
void comp_exit( void );

int ejecutar_programa;	 // 0-Compile, 1-Run, 2-Install

void goto_error( void );

// wwrite falso
void wwrite( byte* copia, int an_copia, int al_copia, int x, int y, int centro, byte* ptr, byte c )
{
	printf( "%s\n", ptr );
}

void wbox( byte* copia, int an_copia, int al_copia, byte c, int x, int y, int an, int al )
{
}

void vuelca_ventana( int m )
{
}

void volcado_copia( void )
{
}

// Copiado de divpcm.cpp
int IsWAV( char* FileName )
{
	FILE* f;
	int	  ok = 1;

	if( ( f = fopen( FileName, "rb" ) ) == NULL ) return ( 0 );

	if( fgetc( f ) != 'R' ) ok = 0;
	if( fgetc( f ) != 'I' ) ok = 0;
	if( fgetc( f ) != 'F' ) ok = 0;
	if( fgetc( f ) != 'F' ) ok = 0;
	fseek( f, 4, SEEK_CUR );
	if( fgetc( f ) != 'W' ) ok = 0;
	if( fgetc( f ) != 'A' ) ok = 0;
	if( fgetc( f ) != 'V' ) ok = 0;
	if( fgetc( f ) != 'E' ) ok = 0;

	fclose( f );

	return ( ok );
}

void _show_items( void )
{
	int n;
	while( n < v.items ) {
		switch( abs( v.item[n].tipo ) ) {
			case 1: printf( "Button: %s\n", v.item[n].button.texto ); break;
			case 2: printf( "Get: %s\n", &v.item[n].get.texto ); break;
			case 3: printf( "Flag: %s\n", &v.item[n].flag.texto ); break;
		}
		n++;
	}
}

int text_len( byte* ptr )
{
	return 0;
}

// copiado de divhelp.cpp
int helpidx[4096];	 // Por cada t‚rmino {inicio,longitud}

void make_helpidx( void )
{
	long  m = -1, n, len;
	byte *help, *i, *help_end;
	FILE* f;

	// i_back = a_back = f_back = 0;	// La cola se vac¡a

	memset( helpidx, 0, sizeof( helpidx ) );
	if( ( f = fopen( "help\\help.div", "rb" ) ) != NULL ) {
		fseek( f, 0, SEEK_END );
		len = ftell( f );
		if( ( help = (byte*)malloc( len ) ) != NULL ) {
			fseek( f, 0, SEEK_SET );
			fread( help, 1, len, f );
			fclose( f );
			help_end = help + len;
			i = help;
			do {
				if( *(word*)i == '{' + '.' * 256 ) {
					len = (long)( i - help );
					n = 0;
					i += 2;
					while( *i >= '0' && *i <= '9' ) n = n * 10 + *i++ - 0x30;
					helpidx[n * 2] = (long)( i + 1 - help );				  // Fija el inicio del t‚rmino n
					if( m >= 0 ) helpidx[m * 2 + 1] = len - helpidx[m * 2];	  // Fija la longitud del anterior
					m = n;
				}
			} while( ++i < help_end );
			if( m >= 0 ) helpidx[m * 2 + 1] = (long)( i - help ) - helpidx[m * 2];
			// free(help);
		}
	}
}

char cerror[128];

extern int linea_error;

void get_error( int n )
{
	FILE* f;
	byte* p;

	sprintf( cerror, texto[381] );
	p = cerror + strlen( cerror );

	if( helpidx[n * 2] && helpidx[n * 2 + 1] ) {
		if( ( f = fopen( "help\\help.div", "rb" ) ) != NULL ) {
			fseek( f, helpidx[n * 2], SEEK_SET );
			fread( p, 1, 128 - strlen( cerror ), f );
			fclose( f );
			while( *p != '}' ) p++;
			*p = 0;
		}
	}

	if( n == 502 || n == 503 || n == 504 || n == 507 ) {
		sprintf( cerror, "%s (%d).", cerror, linea_error );
	}
}

void _process_items( void )
{
}

// copiado de div.cpp
void _button( int t, int x, int y, int c )
{
	v.item[v.items].tipo = 1;
	v.item[v.items].estado = 0;
	v.item[v.items].button.texto = texto[t];
	v.item[v.items].button.x = x;
	v.item[v.items].button.y = y;
	v.item[v.items].button.center = c;
	if( v.selected_item == -1 ) v.selected_item = v.items;
	v.items++;
}

void dialogo( int init_handler )
{
}

void tecla( void )
{
}

void init_compiler_options( t_compiler_options* opts )
{
	opts->max_process = 0;
	opts->ignore_errors = 0;
	opts->free_sintax = 0;
	opts->extended_conditions = 0;
	opts->simple_conditions = 0;
	opts->case_sensitive = 0;
	opts->comprueba_rango = 1;
	opts->comprueba_id = 1;
	opts->comprueba_null = 1;
	opts->hacer_strfix = 1;
	opts->optimizar = 1;
}

// Printf only in verbose mode
void log( const char* fmt, ... )
{
	va_list args;
	if( !verbose ) return;

	va_start( args, fmt );
	vprintf_s( fmt, args );
	va_end( args );
}

void banner()
{
	printf( "DIV Compiler  Copyright (C) 1998,99 Hammer Technologies\n"
			"              Copyright (C) 2016-22 DX GAMES\n"
			"              Copyright (C) 2022 Vii\n"
			"This program is free software: you can redistribute it and/or modify "
			"it under the terms of the GNU General Public License as published by "
			"the Free Software Foundation, either version 3 of the License, or "
			"(at your option) any later version.\n"
			"This program is distributed WITHOUT ANY WARRANTY; without even "
			"the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
			"See the GNU General Public License for more details.\n\n" );
}

void help()
{
	printf( "USAGE: divc [options] FILE\n" );
	printf( "Compiles a DIV program.\n" );
	printf( "\nGeneral options:\n"
			"   -l          Generate EML listing\n"
			"   -d[d]       Generate debug data [and start debugger at program entry]\n"
			"   -v          Be verbose\n"
			"   -h, --help  Show this help\n"
			"\nCompiler options:\n"
			"   --max-process=N        Set maximum number of processes\n"
			"   --extended-conditions  Allow assignments in conditions\n"
			"   --simple-conditions    Allow '=' for comparison in conditions\n"
			"   --case-sensitive       Make keywords and identifiers case sensitive\n"
			"   --ignore-errors        Ignore runtime errors\n"
			"   --free-syntax          Allow simplified syntax\n"
			"   --no-strfix            Don't generate code to auto-fix strings\n"
			"   --no-optimization      Don't optimize generated code\n"
			"   --no-range-check       Disable runtime range checks\n"
			"   --no-id-check          Disable runtime id checks\n"
			"   --no-null-check        Disable runtime null checks\n"
			"   --no-check             no-range-check + no-id-check + no-null-check\n"
			"NOTE: these options don't override the COMPILER_OPTIONS statement.\n" );
}

bool parseOption( t_compiler_options* opts, const char* str )
{
	if( !strncmp( str, "max-process", 11 ) ) {
		if( str[11] != '=' ) {
			fprintf( stderr, "Error: Compiler option max-process requires a value\n" );
			return false;
		}
		opts->max_process = atoi( str + 12 );
		if( opts->max_process <= 0 ) {
			fprintf( stderr, "Error: invalid value for max-process\n" );
			return false;
		}
	} else if( !strcmp( str, "extended-conditions" ) ) {
		opts->extended_conditions = 1;
	} else if( !strcmp( str, "simple-conditions" ) ) {
		opts->simple_conditions = 1;
	} else if( !strcmp( str, "case-sensitive" ) ) {
		opts->case_sensitive = 1;
	} else if( !strcmp( str, "ignore-errors" ) ) {
		opts->ignore_errors = 1;
	} else if( !strcmp( str, "free-syntax" ) ) {
		opts->free_sintax = 1;
	} else if( !strncmp( str, "no-", 3 ) ) {
		str += 3;
		if( !strcmp( str, "strfix" ) ) {
			opts->hacer_strfix = 0;
		} else if( !strcmp( str, "optimization" ) ) {
			opts->optimizar = 0;
		} else if( !strcmp( str, "range-check" ) ) {
			opts->comprueba_rango = 0;
		} else if( !strcmp( str, "id-check" ) ) {
			opts->comprueba_id = 0;
		} else if( !strcmp( str, "null-check" ) ) {
			opts->comprueba_null = 0;
		} else if( !strcmp( str, "check" ) ) {
			opts->comprueba_rango = 0;
			opts->comprueba_id = 0;
			opts->comprueba_null = 0;
		} else {
			str -= 3;
			fprintf( stderr, "Error: Unrecognized compiler option: %s\n", str );
			return false;
		}
	} else {
		if( strcmp( str, "help" ) ) {
			fprintf( stderr, "Error: Unrecognized compiler option: %s\n", str );
		}
		return false;
	}
	return true;
}

void restore_cwd()
{
	chdir( original_cwd );
	free( original_cwd );
}

void chdir_to_exe_dir( char* exe )
{
	char* dir;
	char* path = _fullpath( NULL, exe, 0 );
	if( !path ) return;
	dir = dirname( path );
	chdir( dir );
	free( path );
}

int main( int argc, char* argv[] )
{
	int	  i;
	bool  noMoreOptions = false, listings = false, debug = false, startDebugger = false;
	bool  readOption = false;
	char* p;
	char  file[_MAX_PATH];

	t_compiler_options compiler_options;

	banner();
	file[0] = 0;

	for( i = 1; i < argc; ++i ) {
		if( argv[i][0] == '-' && !noMoreOptions ) {
			if( argv[i][1] == '-' ) {
				if( argv[i][2] == 0 ) {
					noMoreOptions = true;
				} else if( !parseOption( &compiler_options, argv[i] + 2 ) ) {
					help();
					return -1;
				}
				continue;
			}
			for( p = argv[i] + 1; *p; ++p ) {
				switch( *p ) {
					case 'l': listings = true; break;
					case 'd':
						if( debug ) {
							startDebugger = true;
						} else {
							debug = true;
						}
						break;
					case 'c':
						if( readOption ) {
							help();
							return -1;
						}
						readOption = true;
						break;
					case 'v': verbose = 1; break;
					case 'h':
					case '?': help(); return 0;
					default:
						fprintf( stderr, "Error: Unrecognized option: %c\n", *p );
						help();
						return -1;
				}
			}
		} else {
			if( readOption ) {
				readOption = false;
				if( !parseOption( &compiler_options, argv[i] ) ) {
					help();
					return -1;
				}
			} else if( !file[0] ) {
				_fullpath( file, argv[i], sizeof( file ) );
			} else {
				fprintf( stderr, "Error: Too many arguments\n" );
				help();
				return -1;
			}
		}
	}

	if( !file[0] ) {
		fprintf( stderr, "Error: Missing argument: FILE\n" );
		help();
		return -1;
	}

	original_cwd = getcwd( NULL, 0 );
	if( original_cwd ) {
		atexit( restore_cwd );
	}
	chdir_to_exe_dir( argv[0] );

	inicializa_textos( "system\\lenguaje.div" );   // OJO emitir un error si lenguaje.div no existe
	atexit( finaliza_textos );
	make_helpidx();
	inicializa_compilador();   // *** Compilador *** espacios de lower a 00

	return 0;
}
