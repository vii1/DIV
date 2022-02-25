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

// TODO: option to verify checksums in game paks
// TODO: extract individual files. use wildcards? see fnmatch()

#define __STDC_WANT_LIB_EXT1__ 1

#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include "zlib.h"
#include "unpak.h"

int	 verbose = 0;
bool list = false;
bool keepGoing = false;
bool createDirs = false;
bool noLower = false;
uint errors = 0;

Result process_file( char* file, const char* destdir );

char* strlwr_s( char* str, size_t maxsize )
{
	size_t i;
	char*  p;
	for( i = 0, p = str; *p && i < maxsize; ++i, ++p ) {
		*p = (char)tolower( *p );
	}
	return str;
}

char* filename( char* path )
{
	char* p = path + strnlen_s( path, _MAX_PATH ) - 1;
	while( p > path ) {
		--p;
		if( *p == '/' || *p == '\\' || *p == ':' ) {
			return p + 1;
		}
	}
	return path;
}

char* extension( char* path )
{
	char* end = path + strnlen_s( path, _MAX_PATH ) - 1;
	char* p = end;
	while( p > path ) {
		--p;
		if( *p == '.' ) {
			return p;
		} else if( *p == '/' || *p == '\\' || *p == ':' ) {
			return end;
		}
	}
	return end;
}

void banner()
{
	printf( "DIV Unpak  Copyright (C) 2022 Vii\n"
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
	printf( "USAGE: unpak [options] FILE [DESTDIR]\n" );
	printf( "Unpacks DIV install paks and game paks.\n" );
	printf( "Options:\n"
			"   -l      List file info and contents\n"
			"   -d      Create subdirectories when extracting\n"
			"   -u      Don't convert filenames to lower case\n"
			"   -k      Keep going even if file errors are found\n"
			"   -v      Be verbose\n"
			"   -h      Show this help\n"
			"\n" );
}

void log( const char* fmt, ... )
{
	va_list args;
	if( !verbose ) return;

	va_start( args, fmt );
	vprintf_s( fmt, args );
	va_end( args );
}

HeaderType detect_header( FILE* f )
{
	char magic[7];
	if( fread( magic, sizeof( magic ), 1, f ) != 1 ) {
		return HEADER_UNKNOWN;
	} else if( !strcmp( magic, "dat\x1a\x0d\x0a" ) ) {
		log( "File header detected: GAME PACKFILE\n" );
		return HEADER_GAME;
	} else if( !strcmp( magic, "stp\x1a\x0d\x0a" ) ) {
		log( "File header detected: INSTALLER PACKFILE\n" );
		return HEADER_INSTALL;
	}
	return HEADER_UNKNOWN;
}

// Switches extension to .001, then process that file
Result process_first_volume( char* file, char* ext, const char* destdir )
{
	char*  first;
	size_t len;
	Result result;
	*ext = 0;
	len = strnlen_s( file, _MAX_PATH );
	first = (char*)malloc( len + 5 );
	memcpy_s( first, len, file, len );
	memcpy_s( first + len, 5, ".001", 5 );
	result = process_file( first, destdir );
	free( first );
	return result;
}

Result process_file( char* file, const char* destdir )
{
	FILE*	   f = NULL;
	HeaderType headerType;

	log( "Opening file: %s\n", file );
	f = fopen( file, "rb" );
	if( !f ) {
		perror( file );
		return ERR_IO;
	}
	headerType = detect_header( f );
	if( headerType == HEADER_GAME ) {
		PakInfo* info = NULL;
		Result	 result = read_pak_info( f, headerType, &info );
		if( !list && result == ERR_OK ) {
			result = extract( f, file, info, 0, destdir );
		} else {
			fclose( f );
		}
		free_PakInfo( info );
		return result;
	} else if( headerType == HEADER_INSTALL ) {
		char* ext = extension( file );
		if( strcmp( ext, ".001" ) ) {
			fclose( f );
			return process_first_volume( file, ext, destdir );
		} else {
			PakInfo* info = NULL;
			Result	 result = read_pak_info( f, headerType, &info );
			if( !list && result == ERR_OK ) {
				// TODO: extract
			}
			free_PakInfo( info );
			fclose( f );
			return result;
		}
	} else {
		fprintf_s( stderr, "Unrecognized file format\n" );
		return ERR_FILE_FORMAT;
	}
}

int main( int argc, char* argv[] )
{
	bool  hlp = false, noMoreOptions = false;
	char *file = NULL, *destdir = NULL;
	int	  i;
	char* p;

	banner();
	if( argc < 2 ) {
		help();
		return ERR_COMMAND_LINE;
	}

	for( i = 1; i < argc; ++i ) {
		if( argv[i][0] == '-' && !noMoreOptions ) {
			if( argv[i][1] == '-' && argv[i][2] == 0 ) {
				noMoreOptions = true;
				continue;
			}
			for( p = argv[i] + 1; *p; ++p ) {
				switch( *p ) {
					case 'l': list = true; break;
					case 'd': createDirs = true; break;
					case 'u': noLower = true; break;
					case 'k': keepGoing = true; break;
					case 'v': verbose = 1; break;
					case 'h': hlp = true; break;
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

	if( !file || ( list && destdir ) ) {
		help();
		return ERR_COMMAND_LINE;
	}

	return process_file( file, destdir );
}
