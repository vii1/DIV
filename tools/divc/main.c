#include <stdio.h>

#define DEFINIR_AQUI
#include "global.h"
#undef DEFINIR_AQUI

// copiado de div.cpp
byte lower[256] = "                                   #$           0123456789      "
				  " abcdefghijklmnopqrstuvwxyz    _ abcdefghijklmnopqrstuvwxyz     "
				  "áueaaaaáeeeiiiaaeëëooouuyouõúùûüaiou§§¶ß                        "
				  "                                                                ";

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
int helpidx[4096];	 // Por cada tÇrmino {inicio,longitud}

void make_helpidx( void )
{
	long  m = -1, n, len;
	byte *help, *i, *help_end;
	FILE* f;

	// i_back = a_back = f_back = 0;	// La cola se vac°a

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
					helpidx[n * 2] = (long)( i + 1 - help );				  // Fija el inicio del tÇrmino n
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

void banner()
{
	printf( "DIV Compiler  Copyright (C) 1998,99 Hammer Technologies\n"
			"			   Copyright (C) 2016-22 DX GAMES\n"
			"			   Copyright (C) 2022 Vii\n"
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
	printf( "Options:\n"
			"   -l         Generate EML listing\n"
			"   -d[d]      Generate debug data [and start debugger at program entry]\n"
			"   -c OPT[=N] Set a compiler option:\n"
			"              -c max_process=N       Set maximum number of processes\n"
			"              -c extended_conditions Allow assignments in conditions\n"
			"              -c simple_conditions   Allow '=' for comparison in conditions\n"
			"              -c case_sensitive      Make keywords and identifiers case sensitive\n"
			"              -c ignore_errors       Ignore runtime errors\n"
			"              -c free_syntax         Allow simplified syntax\n"
			"              -c no_strfix           Don't generate code to auto-fix strings\n"
			"              -c no_optimization     Don't optimize generated code\n"
			"              -c no_range_check      Disable runtime range checks\n"
			"              -c no_id_check         Disable runtime id checks\n"
			"              -c no_null_check       Disable runtime null checks\n"
			"              -c no_check            no_range_check + no_id_check + no_null_check\n"
			"              NOTE: these options don't override the COMPILER_OPTIONS statement.\n"
			"   -v         Be verbose\n"
			"   -h         Show this help\n"
			"\n" );
}

int main( int argc, char* argv[] )
{
	banner();
	inicializa_textos( "system\\lenguaje.div" );   // OJO emitir un error si lenguaje.div no existe
	make_helpidx();

	if(argc < 2) {
		help();
		return -1;
	}
}