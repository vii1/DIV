#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char install_dir[_MAX_PATH + 1];

void chdir_to_install_dir() {
	int	 n;
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

	if( argc < 1 ) return 0;
    
	// TODO: considerar un handler que sirva para algo
	//  _harderr(critical_error);

	chdir_to_install_dir();

	return 0;
}