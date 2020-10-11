#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _MAX_PATH
#	ifndef MAX_PATH
#		define MAX_PATH _MAX_PATH
#	endif
#endif
#ifndef MAX_PATH
#	define MAX_PATH 144
#endif

#define MAX_COLS 80

const char* basename( const char* path )
{
	const char* p = path + strlen( path ) - 1;
	for( ; p >= path; --p ) {
		if( *p == '/' || *p == '\\' ) {
			return p + 1;
		}
	}
	return path;
}

char* stripext( char* dst, const char* filename )
{
	const char* p = filename + strlen( filename ) - 1;
	for( ; p > filename; --p ) {
		if( *p == '.' ) {
			size_t size = p - filename;
			memcpy( dst, filename, size - 1 );
			dst[size - 1] = 0;
			return dst;
		}
	}
	strcpy( dst, filename );
	return dst;
}

const char* error()
{
	static char errorString[256];
	strerror_s( errorString, sizeof( errorString ), errno );
	return errorString;
}

int main( int argc, char* argv[] )
{
	const char* binario = argv[1];
	char		simbolo[MAX_PATH];
	FILE*		f;
	size_t		col;
	int			coma = 0;
	if( argc < 2 || argc > 3 ) {
		printf( "USO: bin2h <fichero binario> [nombre del s¡mbolo]\n" );
		return 1;
	}

	if( argc > 2 ) {
		strcpy_s( simbolo, MAX_PATH, argv[2] );
	} else {
		stripext( simbolo, basename( binario ) );
	}

	f = fopen( binario, "rb" );
	if( !f ) {
		fprintf( stderr, "Error al leer %s: %s\n", binario, error() );
	}
	printf( "static const char %s[] = {", simbolo );
	col = 18 + 6 + strlen( simbolo );
	while( !feof( f ) ) {
		unsigned char c		  = (unsigned char)fgetc( f );
		int			  addcols = coma;
		if( c > 99 ) {
			addcols += 3;
		} else if( c > 9 ) {
			addcols += 2;
		} else {
			addcols += 1;
		}
		if( coma ) printf( "," );
		if( col + addcols >= MAX_COLS ) {
			printf( "\n" );
			col = addcols - coma;
		} else {
			col += addcols;
		}
		printf( "%d", c );
		if( !coma ) coma = 1;
	}
	fclose( f );
	if( col + 2 >= MAX_COLS ) printf( "\n" );
	printf( "};\n" );
	return 0;
}
