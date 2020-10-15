#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main( int argc, char* argv[] )
{
	char full[_MAX_PATH + 1];
	int	 n;

	if( argc < 1 ) return 0;
	_fullpath( full, argv[0], _MAX_PATH + 1 );
	n = strlen( full );
	while( n && full[n] != '\\' ) n--;
	full[n] = 0;
	if( full[n - 1] == ':' ) strcat( full, "\\" );
	_dos_setdrive( (int)toupper( full[0] ) - 'A' + 1, &n );
	chdir( full );
    
	return 0;
}