#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include <direct.h>

#include <zlib.h>

#include "main.h"
#include "video.h"
#include "fpg.h"
#include "fnt.h"

struct {
	const char* appName;
	const char* copyright;
	const char* packName;
	const char* defaultDir;
	const char* msgFin;
	const char* msgHelp;
	const char* msgDisk1;
	const char* msgDisk2;
	const char* errores[9];
	dword		totalLen;
	dword		packSize;
	int			createDir;
	int			includeSetup;
	int			segundoFont;
	dword		sizeInstallFpgZ;
	dword		sizeInstallFpg;
	dword		sizeSmallFntZ;
	dword		sizeSmallFnt;
	dword		sizeBigFntZ;
	dword		sizeBigFnt;
} installData = { 0 };

char install_dir[_MAX_PATH + 1];

void error( ErrorNum num )
{
	printf( "%s\n", installData.errores[num] );
	exit( 1 );
}

const char* next_string( const char* prevString )
{
	return prevString + strlen( prevString ) + 1;
}

char* div_strdup( const char* string )
{
	char* s = strdup( string );
	if( !s ) {
		video_reset();
		if( installData.errores[E_MEMORIA] ) {
			printf( "%s\n", installData.errores[E_MEMORIA] );
		} else {
			printf( "Not enough memory.\n" );
		}
		exit( 1 );
	}
	return s;
}

void* div_malloc( size_t size )
{
	void* p = malloc( size );
	if( !p ) {
		video_reset();
		if( installData.errores[E_MEMORIA] ) {
			printf( "%s\n", installData.errores[E_MEMORIA] );
		} else {
			printf( "Not enough memory.\n" );
		}
		exit( 1 );
	}
	return p;
}

void descomprimir( byte* dest, unsigned int destLen, const byte* src, unsigned int srcLen )
{
	switch( uncompress( dest, (unsigned long*)&destLen, src, srcLen ) ) {
		case Z_OK: break;
		case Z_MEM_ERROR: {
			error( E_MEMORIA );
		}
		default: {
			error( E_FPG );
		}
	}
}

void lee_datos_exe( const char* exe )
{
	FILE*		f;
	int			i, n;
	char*		buf;
	int*		iptr;
	const char* ptr;

	f = fopen( exe, "rb" );
	if( !f ) {
		printf( "EXE file damaged.\n" );
		exit( 1 );
	}
	fseek( f, -4, SEEK_END );
	fread( &n, 1, 4, f );
	fseek( f, -28, SEEK_END );
	fread( &installData.sizeInstallFpgZ, 1, 4, f );
	fread( &installData.sizeInstallFpg, 1, 4, f );
	fread( &installData.sizeSmallFntZ, 1, 4, f );
	fread( &installData.sizeSmallFnt, 1, 4, f );
	fread( &installData.sizeBigFntZ, 1, 4, f );
	fread( &installData.sizeBigFnt, 1, 4, f );
	fseek( f, -n, SEEK_END );
	buf = (char*)div_malloc( n );
	fread( buf, n, 1, f );
	fclose( f );
	ptr = buf;
	installData.appName = div_strdup( ptr );
	ptr = next_string( ptr );
	installData.copyright = div_strdup( ptr );
	ptr = next_string( ptr );
	installData.packName = div_strdup( ptr );
	ptr = next_string( ptr );
	installData.defaultDir = div_strdup( ptr );
	ptr = next_string( ptr );
	installData.msgFin = div_strdup( ptr );
	ptr = next_string( ptr );
	installData.msgHelp = div_strdup( ptr );
	ptr = next_string( ptr );
	installData.msgDisk1 = div_strdup( ptr );
	ptr = next_string( ptr );
	installData.msgDisk2 = div_strdup( ptr );
	ptr = next_string( ptr );
	for( i = 0; i < 9; ++i ) {
		installData.errores[i] = div_strdup( ptr );
		ptr = next_string( ptr );
	}
	iptr = (int*)ptr;
	installData.totalLen = iptr[0];
	installData.packSize = iptr[1];
	installData.createDir = iptr[2];
	installData.includeSetup = iptr[3];
	installData.segundoFont = iptr[4];

	ptr = (byte*)&iptr[5];
	cargar_fpg( ptr, installData.sizeInstallFpg, installData.sizeInstallFpgZ );
	ptr += installData.sizeInstallFpgZ;
	cargar_fnt( &smallFnt, ptr, installData.sizeSmallFnt, installData.sizeSmallFntZ );
	if( installData.segundoFont && installData.sizeBigFntZ ) {
		ptr += installData.sizeSmallFntZ;
		cargar_fnt( &bigFnt, ptr, installData.sizeBigFnt, installData.sizeBigFntZ );
	} else {
		bigFnt = smallFnt;
	}
	free( buf );
}

void chdir_to_install_dir( const char* argv0 )
{
	unsigned int n;
	_fullpath( install_dir, argv0, _MAX_PATH + 1 );
	n = strlen( install_dir );
	while( n && install_dir[n] != '\\' ) n--;
	install_dir[n] = 0;
	if( install_dir[n - 1] == ':' ) strcat( install_dir, "\\" );
	_dos_setdrive( (int)toupper( install_dir[0] ) - 'A' + 1, &n );
	chdir( install_dir );
}

int main( int argc, char* argv[] )
{
#ifdef _DEBUG
	if( 0 ) {
		argv[0] = "D:\\BUILD.DOS\\INSTALL\\DEBUG\\TEST\\INSTALL.EXE";
	}
#endif
	if( argc < 1 ) return 1;

	// TODO: considerar un handler que sirva para algo
	//  _harderr(critical_error);

	chdir_to_install_dir( argv[0] );
	lee_datos_exe( argv[0] );

	// TODO: obtener discos duros y espacio libre
	verificar_fpg();

	if( video_set_mode() ) {
		error( E_VESA );
	}
	pal_init();

	// Imagen intro
	volcado( fpg_map( 2 ) );
	fade_on();
	
	fade_off();
	video_reset();
	return 0;
}
