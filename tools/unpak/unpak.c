/*
This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "zlib.h"

enum { ERR_OK = 0, ERR_COMMAND_LINE = 1, ERR_IO = 2, ERR_FILE_FORMAT = 3, ERR_OTHER = 4 };

int verbose = 0;

void banner()
{
	printf( "DIV Unpak  Copyright (C) 2022 Vii\n"
			"This program is free software: you can redistribute it and/or modify "
			"it under the terms of the GNU General Public License as published by "
			"the Free Software Foundation, either version 3 of the License, or "
			"(at your option) any later version.\n"
			"This program is distributed WITHOUT ANY WARRANTY; without even "
			"the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
			"See the GNU General Public License for more details.\n" );
}

void help()
{
	printf( "\nUSAGE: unpak [options] FILE [DESTDIR]\n" );
	printf( "Unpacks DIV install paks and game paks.\n" );
	printf( "Options:\n"
			"   -l      List file info and contents\n"
			"   -v      Be verbose\n"
			"   -h      Show this help\n"
			"\n" );
}

int main( int argc, char* argv[] )
{
	int	  list = 0, hlp = 0;
	char *file = NULL, *destdir = NULL;
	int	  i;
	char* p;

	banner();
	if( argc < 2 ) {
		help();
		return ERR_COMMAND_LINE;
	}

	for( i = 1; i < argc; ++i ) {
		if( argv[i][0] == '-' ) {
			for( p = argv[i] + 1; *p; ++p ) {
				switch( *p ) {
					case 'l': list = 1; break;
					case 'v': verbose = 1; break;
					case 'h': hlp = 1; break;
					default: help(); return ERR_COMMAND_LINE;
				}
			}
		} else {
			if( !file ) {
				file = argv[i];
			} else if( !destdir ) {
				destdir = argv[i];
			} else {
				help();
				return ERR_COMMAND_LINE;
			}
		}
	}

	if( hlp ) {
		help();
		return ERR_OK;
	}

	if( !file ) {
		help();
		return ERR_COMMAND_LINE;
	}
}