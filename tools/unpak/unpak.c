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

#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>
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
	uint	  crc[3];	  // Checksum (not true CRC) for compiled exes: 0=game exe, 1=setup.exe, 2=reserved
	uint	  numFiles;	  // Number of files in packfile
	FileInfo* files;	  // File info data
} GamePakInfo;

int	 verbose = 0;
bool list = false;
bool keepGoing = false;
bool createDirs = false;
bool noLower = false;

char* strlwr_s( char* str, size_t maxsize )
{
	size_t i;
	char*  p;
	for( i = 0, p = str; *p && i < maxsize; ++i, ++p ) {
		*p = (char)tolower( *p );
	}
	return str;
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

void free_GamePakInfo( GamePakInfo* p )
{
	if( p ) {
		if( p->files ) {
			free( p->files );
		}
		free( p );
	}
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
	uint		 i, totalZSize = 0, totalUSize = 0;
	FileInfo*	 fi;
	float		 ratio;

	assert( f );
	assert( pInfo );

	if( !info ) err( ERR_MEMORY );
	info->files = NULL;
	// Version must be 0
	if( fread( &info->version, 1, 1, f ) != 1 ) err( ERR_IO );
	log( "File version: %u\n", info->version );
	if( info->version != 0 ) {
		if( keepGoing ) {
			fprintf_s( stderr, "WARNING: File version is %u (expected 0), probably unsupported\n", info->version );
		} else {
			fprintf_s( stderr, "ERROR: File version is %u (expected 0)\n", info->version );
			fprintf_s( stderr, "To ignore this error, use -k option\n" );
			err( ERR_VERSION );
		}
	}
	// Checksums for exes
	for( i = 0; i < 3; ++i ) {
		if( fread( &info->crc[i], 4, 1, f ) != 1 ) err( ERR_IO );
		log( "Checksum #%u: 0x%08X\n", i, info->crc[i] );
	}
	// Number of files in packfile
	if( fread( &info->numFiles, 4, 1, f ) != 1 ) err( ERR_IO );
	log( "Number of files: %u\n", info->numFiles );
	info->files = (FileInfo*)malloc( sizeof( FileInfo ) * info->numFiles );
	if( !info->files ) err( ERR_MEMORY );
	// List header
	if( list && !verbose ) {
		printf_s( "Uncomp.size  Comp.size  Ratio Name\n"
				  "----------- ---------- ------ ----------------\n" );
	}
	// Read file infos
	fi = info->files;
	for( i = 0; i < info->numFiles; ++i, ++fi ) {
		if( fread( &fi->name, 16, 1, f ) != 1 ) err( ERR_IO );
		if( strnlen_s( fi->name, 16 ) == 16 ) {
			if( keepGoing ) {
				fprintf_s( stderr, "WARNING: File #%u (%.16s): name with no zero terminator\n", i, fi->name );
			} else {
				fprintf_s( stderr, "ERROR: File #%u (%.16s): name with no zero terminator\n", i, fi->name );
				fprintf_s( stderr, "To ignore this error, use -k option\n" );
				err( ERR_FILE_FORMAT );
			}
		}
		if( !noLower ) {
			strlwr_s( fi->name, 16 );
		}
		if( fread( &fi->offset, 4, 1, f ) != 1 ) err( ERR_IO );
		if( fread( &fi->zSize, 4, 1, f ) != 1 ) err( ERR_IO );
		totalZSize += fi->zSize;
		if( fread( &fi->uSize, 4, 1, f ) != 1 ) err( ERR_IO );
		totalUSize += fi->uSize;
		if( fi->uSize == 0 ) {
			ratio = _INFF;
		} else {
			ratio = ( (float)fi->zSize / fi->uSize ) * 100.f;
		}
		if( verbose ) {
			int	 ndigits = 0;
			uint n = info->numFiles;
			while( n ) {
				++ndigits;
				n /= 10;
			}
			log( "%*u: %-16.16s offset: %-10u uSize: %-10u zSize: %-10u ratio: %3.1f%%\n", ndigits, i, fi->name, fi->offset,
			  fi->uSize, fi->zSize, ratio );
		} else if( list ) {
			printf_s( " %10u %10u %5.1f%% %-.16s\n", fi->uSize, fi->zSize, ratio, fi->name );
		}
	}
	if( totalUSize == 0 ) {
		ratio = _INFF;
	} else {
		ratio = ( (float)totalZSize / totalUSize ) * 100.f;
	}
	if( verbose ) {
		printf_s( "TOTAL: uSize: %-10u zSize: %-10u ratio: %.1f%%\n", totalUSize, totalZSize, ratio );
	}
	if( list && !verbose ) {
		printf_s( "----------- ---------- ------ ----------------\n"
				  " %10u %10u %5.1f%% %u file(s) total\n",
		  totalUSize, totalZSize, ratio, info->numFiles );
	}
	*pInfo = info;
	return ERR_OK;

_error:
	*pInfo = NULL;
	free_GamePakInfo( info );
	if( result == ERR_MEMORY || result == ERR_IO ) {
		perror( "Fatal" );
	}
	return result;
}

#undef err

Result process_file( const char* file, const char* destdir )
{
	FILE*	   f = NULL;
	HeaderType headerType;

	log( "Opening file: %s\n", file );
	f = fopen( file, "rb" );
	if( !f ) {
		perror( "Error opening file" );
		return ERR_IO;
	}
	headerType = detect_header( f );
	if( headerType == HEADER_GAME ) {
		GamePakInfo* info = NULL;
		Result		 result = read_game_info( f, &info );
		if( list || result != ERR_OK ) {
			fclose( f );
			return result;
		}
	} else if( headerType == HEADER_INSTALL ) {
	} else {
		fclose( f );
		fprintf_s( stderr, "Unrecognized file format\n" );
		return ERR_FILE_FORMAT;
	}

	fclose( f );
	return ERR_OK;
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
