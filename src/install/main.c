#ifdef __cplusplus
#	undef __cplusplus
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum _ErrorNum {
	E_RUTA = 0, // La unidad o ruta especificada no es v lida
	E_INTERR = 1, // La instalaci¢n ha sido interrumpida por el usuario
	E_LECTURA = 2, // Se produjo un error en la lectura de los archivos
	E_ESCRITURA = 3, // Se produjo un error en la escritura de los archivos
	E_ESPACIO = 4, // No hay suficiente espacio en la unidad seleccionada
	E_MEMORIA = 5, // No hay suficiente memoria para realizar la instalaci¢n
	E_FPG = 6, // El FPG interno no es v lido (faltan gr ficos o puntos de control)
	E_VESA = 7, // No se encontr¢ un driver VESA (controlador de pantalla)
	E_DISCO = 8 // No se encontr¢ ning£n disco duro en el que instalar
};

typedef struct {
	const char* buf;
	const char* appName;
	const char* copyright;
	const char* packName;
	const char* defaultDir;
	const char* msgFin;
	const char* msgHelp;
	const char* msgDisk1;
	const char* msgDisk2;
	const char* errores[9];
	int			totalLen;
	int			packSize;
	int			createDir;
	int			includeSetup;
	int			segundoFont;
	int			size[6];
} InstallData;

char install_dir[_MAX_PATH + 1];

const char* next_string( const char* prevString )
{
	return prevString + strlen( prevString ) + 1;
}

void lee_datos_exe( const char* exe, InstallData* data )
{
	FILE* f;
	int	  i, n;
	char* buf;
	int*  iptr;

	f = fopen( exe, "rb" );
	if( !f ) {
		printf( "EXE file damaged.\n" );
		exit( 1 );
	}
	fseek( f, -4, SEEK_END );
	fread( &n, 1, 4, f );
	fseek( f, -28, SEEK_END );
	fread( data->size, 6, 4, f );
	fseek( f, -n, SEEK_END );
	buf = (char*)malloc( n );
	if( !buf ) {
		printf( "Not enough memory.\n" );
		exit( 1 );
	}
	fread( buf, n, 1, f );
	data->buf = buf;
	data->appName = buf;
	data->copyright = next_string( data->appName );
	data->packName = next_string( data->copyright );
	data->defaultDir = next_string( data->packName );
	data->msgFin = next_string( data->defaultDir );
	data->msgHelp = next_string( data->msgFin );
	data->msgDisk1 = next_string( data->msgHelp );
	data->msgDisk2 = next_string( data->msgDisk1 );
	data->errores[0] = next_string( data->msgDisk2 );
	for( i = 1; i < 9; ++i ) {
		data->errores[i] = next_string( data->errores[i - 1] );
	}
	fseek( f, -n + (int)( next_string( data->errores[8] ) - data->buf ), SEEK_END );
	fread( &data->totalLen, 1, 4, f );
	fread( &data->packSize, 1, 4, f );
	fread( &data->createDir, 1, 4, f );
	fread( &data->includeSetup, 1, 4, f );
	fread( &data->segundoFont, 1, 4, f );
	fclose( f );
}

void chdir_to_install_dir()
{
	int n;
	_fullpath( install_dir, argv[0], _MAX_PATH + 1 );
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

	chdir_to_install_dir();

	return 0;
}