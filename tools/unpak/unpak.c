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

#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include "zlib.h"

typedef unsigned char byte;
typedef unsigned int  uint;

// Result codes
typedef enum {
	ERR_OK = 0,			// Successful
	ERR_COMMAND_LINE,	// Error in command line
	ERR_IO,				// I/O error or file not found
	ERR_FILE_FORMAT,	// Invalid file format
	ERR_VERSION,		// Invalid file version
	ERR_MEMORY,			// Out of memory
	ERR_OTHER,			// Other
} Result;

// Type of file header
typedef enum {
	HEADER_UNKNOWN = 0,	  // Unknown / unrecognized
	HEADER_INSTALL,		  // Installer packfile
	HEADER_GAME,		  // Game packfile
} HeaderType;

// Common: Fileinfo struct
typedef struct {
	char   name[16];   // File name
	size_t offset;	   // File data offset in packfile
	size_t zSize;	   // Compressed file size (if ==uSize, it's not compressed)
	size_t uSize;	   // Uncompressed file size
} FileInfo;

// Installer only: Packfile struct
typedef struct {
	bool	  isFinalVolume;   // Is this the last volume?
	uint	  numFiles;		   // Number of files in packfile
	FileInfo* files;		   // File info data
} InstallPakInfo;

// Game only: Packfile struct
typedef struct {
	byte	  version;	  // File format version
	uint	  crc[3];	  // CRC checksum
	uint	  numFiles;	  // Number of files in packfile
	FileInfo* files;	  // File info data
} GamePakInfo;

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
			"See the GNU General Public License for more details.\n\n" );
}

void help()
{
	printf( "USAGE: unpak [options] FILE [DESTDIR]\n" );
	printf( "Unpacks DIV install paks and game paks.\n" );
	printf( "Options:\n"
			"   -l      List file info and contents\n"
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

#define err( x )     \
	{                \
		result = x;  \
		goto _error; \
	}

Result read_game_info( FILE* f, GamePakInfo** pInfo )
{
	Result		 result = ERR_OTHER;
	GamePakInfo* info = (GamePakInfo*)malloc( sizeof( GamePakInfo ) );
	if( !info ) err( ERR_MEMORY );
	info->files = NULL;
	if( !fread( &info->version, 1, 1, f ) != 1 ) err( ERR_IO );
	if( info->version != 0 ) err( ERR_VERSION );

	return ERR_OK;

_error:
	*pInfo = NULL;
	if( info ) {
		if( info->files ) {
			free( info->files );
		}
		free( info );
	}
	return result;
}

Result process_file( const char* file, const char* destdir, bool list, bool keepGoing )
{
	FILE* f;
	HeaderType headerType;

	log( "Opening file: %s\n", file );
	f = fopen( file, "rb" );
	if( !f ) {
		perror( "Error opening file" );
		return ERR_IO;
	}
	headerType = detect_header( f );
	if( headerType == HEADER_UNKNOWN ) {
		fclose( f );
		fprintf_s( stderr, "Unrecognized file format\n" );
		return ERR_FILE_FORMAT;
	}

	if( headerType == HEADER_GAME ) {
	}
	return ERR_OK;
}

int main( int argc, char* argv[] )
{
	bool  list = false, hlp = false, keepGoing = false;
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
					case 'l': list = true; break;
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

	if( !file || ( list && !destdir ) ) {
		help();
		return ERR_COMMAND_LINE;
	}

	return process_file( file, destdir, list, keepGoing );
}
