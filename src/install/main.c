#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include <direct.h>

#include <zlib.h>

enum _ErrorNum {
	E_RUTA = 0,		   // La unidad o ruta especificada no es v lida
	E_INTERR = 1,	   // La instalaci¢n ha sido interrumpida por el usuario
	E_LECTURA = 2,	   // Se produjo un error en la lectura de los archivos
	E_ESCRITURA = 3,   // Se produjo un error en la escritura de los archivos
	E_ESPACIO = 4,	   // No hay suficiente espacio en la unidad seleccionada
	E_MEMORIA = 5,	   // No hay suficiente memoria para realizar la instalaci¢n
	E_FPG = 6,		   // El FPG interno no es v lido (faltan gr ficos o puntos de control)
	E_VESA = 7,		   // No se encontr¢ un driver VESA (controlador de pantalla)
	E_DISCO = 8		   // No se encontr¢ ning£n disco duro en el que instalar
};
typedef enum _ErrorNum ErrorNum;

struct {
	const char*	 appName;
	const char*	 copyright;
	const char*	 packName;
	const char*	 defaultDir;
	const char*	 msgFin;
	const char*	 msgHelp;
	const char*	 msgDisk1;
	const char*	 msgDisk2;
	const char*	 errores[9];
	unsigned int totalLen;
	unsigned int packSize;
	int			 createDir;
	int			 includeSetup;
	int			 segundoFont;
	unsigned int sizeInstallFpgZ;
	unsigned int sizeInstallFpg;
	unsigned int sizeSmallFntZ;
	unsigned int sizeSmallFnt;
	unsigned int sizeBigFntZ;
	unsigned int sizeBigFnt;
} installData = { 0 };

typedef struct {
	unsigned int code;
	unsigned int length;
	char		 description[32];
	char		 filename[12];
	unsigned int width;
	unsigned int height;
	unsigned int numCpoints;
	_Packed struct {
		short x, y;
	} cpoints[];
} FpgMapHeader;

typedef struct {
	unsigned int width;
	unsigned int height;
	int			 inc_y;
	unsigned int offset;
} FntCharHeader;

typedef struct {
	unsigned char* fnt;
	FntCharHeader* table;
	unsigned int   averageWidth;
	unsigned int   maxHeight;
	unsigned int   averageCenter;
} Fnt;

char		   install_dir[_MAX_PATH + 1];
unsigned char* paleta;
FpgMapHeader** fpgIndex;
unsigned char* installFpg;
Fnt			   smallFnt;
Fnt			   bigFnt;

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
		if( installData.errores[E_MEMORIA] ) {
			printf( "%s\n", installData.errores[E_MEMORIA] );
		} else {
			printf( "Not enough memory.\n" );
		}
		exit( 1 );
	}
	return p;
}

void descomprimir( unsigned char* dest, unsigned int destLen, const unsigned char* src, unsigned int srcLen )
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

void cargar_fpg( const unsigned char* buf, unsigned int uSize, unsigned int zSize )
{
	unsigned char* ptr;
	int			   i = 0;
	unsigned int   max = 0;
	installFpg = div_malloc( uSize );
	descomprimir( installFpg, uSize, buf, zSize );
	if( memcmp( installFpg, "fpg\x1a\x0d\x0a", 7 ) ) {
		error( E_FPG );
	}
	paleta = installFpg + 8;
	ptr = installFpg + 0x548;
	while( ptr < installFpg + uSize && i < 1000 ) {
		FpgMapHeader* map = (FpgMapHeader*)ptr;
		if( map->code > max ) max = map->code;
		++i;
		ptr += map->length;
	}
	fpgIndex = div_malloc( max * sizeof( FpgMapHeader* ) );
	memset( fpgIndex, 0, max * sizeof( FpgMapHeader* ) );
	ptr = installFpg + 0x548;
	i = 0;
	while( ptr < installFpg + uSize && i < 1000 ) {
		FpgMapHeader* map = (FpgMapHeader*)ptr;
		fpgIndex[map->code] = map;
		++i;
		ptr += map->length;
	}
}

void cargar_fnt( Fnt* fnt, const unsigned char* buf, unsigned int uSize, unsigned int zSize )
{
	FntCharHeader* i;
	int			   numChars = 0;
	int			   totalWidth = 0;

	fnt->fnt = div_malloc( uSize );
	descomprimir( fnt->fnt, uSize, buf, zSize );
	if( memcmp( fnt->fnt, "fnt\x1a\x0d\x0a", 7 ) ) {
		error( E_FPG );
	}
	fnt->table = (FntCharHeader*)( fnt->fnt + 0x54c );
	fnt->maxHeight = 0;
	for( i = fnt->table; i < fnt->table + 256; ++i ) {
		if( i->width ) {
			++numChars;
			totalWidth += i->width;
			if( i->inc_y + i->height > fnt->maxHeight ) {
				fnt->maxHeight = i->inc_y + i->height;
			}
		}
	}
	fnt->averageWidth = totalWidth / numChars;
	fnt->averageCenter = fnt->averageWidth / 2;
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

	ptr = (unsigned char*)&iptr[5];
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
	if( argc < 1 ) return 1;

	// TODO: considerar un handler que sirva para algo
	//  _harderr(critical_error);

	chdir_to_install_dir( argv[0] );
	lee_datos_exe( argv[0] );

	return 0;
}
