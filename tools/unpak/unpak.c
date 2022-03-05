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

#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>
#include <direct.h>
#include <io.h>
#include "unpak.h"

int	 verbose = 0;
bool list = false;
bool keepGoing = false;
bool keepBroken = false;
bool createDirs = false;
bool noLower = false;
uint errors = 0;

Result process_file( char* file, char* destdir, const char** patterns, uint numPatterns );

// A more secure strlwr()
char* strlwr_s( char* str, size_t maxsize )
{
	size_t i;
	char*  p;
	for( i = 0, p = str; *p && i < maxsize; ++i, ++p ) {
		*p = (char)tolower( *p );
	}
	return str;
}

// Returns a pointer to the beginning of the filename in a path
// If there is no filename, returns a pointer to the ending zero (empty string)
char* filename( char* path )
{
	char* p = path + strnlen_s( path, _MAX_PATH );
	while( p > path ) {
		--p;
		if( *p == '/' || *p == '\\' || *p == ':' ) {
			return p + 1;
		}
	}
	return path;
}

// Returns a pointer to the beginning of the extension in a path (more precisely, the last dot)
// If there is no extension, returns a pointer to the ending zero (empty string)
char* extension( char* path )
{
	char* end = path + strnlen_s( path, _MAX_PATH );
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
	printf( "USAGE: unpak [options] FILE [PATTERN...]\n" );
	printf( "Unpacks DIV install paks and game paks. Use PATTERNs to extract/list only matching files.\n" );
	printf( "Options:\n"
			"   -l      List file info and contents\n"
			"   -d DIR  Extract files into specified directory\n"
			"   -s      Create subdirectories when extracting\n"
			"   -u      Don't convert filenames to lower case\n"
			"   -k      Keep going even if file errors are found\n"
			"   -b      Don't delete partially extracted or corrupted files\n"
			"   -v      Be verbose\n"
			"   -h      Show this help\n"
			"\n" );
}

// Printf only in verbose mode
void log( const char* fmt, ... )
{
	va_list args;
	if( !verbose ) return;

	va_start( args, fmt );
	vprintf_s( fmt, args );
	va_end( args );
}

// Detect kind of packfile
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

// Alters filename setting its extension to the corresponding volume number
// Example: set_volume_extension( "hello.001", 49 ) -> "hello.049"
// PRE: filename is mutable and ends in a 3-character extension
void set_volume_extension( char* filename, uint volume )
{
	char* p = strrchr( filename, '.' );
	if( p ) {
		snprintf_s( p + 1, 4, "%3.3u", volume );
	}
}

// Switches extension to .001, then process that file
// PRE: ext is the result of using extension() on file
// TODO: check for path too long
Result process_first_volume( char* file, char* ext, char* destdir, const char** patterns, uint numPatterns )
{
	char   first[_MAX_PATH];
	size_t len = min( ext - file, _MAX_PATH - 5 );
	memcpy_s( first, _MAX_PATH, file, len );
	memcpy_s( first + len, 5, ".001", 5 );
	return process_file( first, destdir, patterns, numPatterns );
}

// Checks destdir exists and is a directory
// If not, tries to mkdir it (not recursive)
Result check_destdir( char* destdir )
{
	struct stat st;
	char*		p = destdir + strnlen_s( destdir, _MAX_PATH ) - 1;
	// Trims every \ / from the end
	while( p > destdir && ( *p == '/' || *p == '\\' ) ) {
		*p = 0;
		--p;
	}
	// Check destdir exists (may be a file)
	if( access( destdir, 0 ) ) {
		if( errno == ENOENT ) {
			// Does not exist. Try to create it
			if( mkdir( destdir ) )
				return ERR_IO;
			else
				return ERR_OK;
		} else {
			// Access denied.
			return ERR_IO;
		}
	}
	// It exists. Let's make sure it's a directory.
	if( stat( destdir, &st ) ) return ERR_IO;
	if( !S_ISDIR( st.st_mode ) ) return ERR_COMMAND_LINE;
	return ERR_OK;
}

// Does the thing
Result process_file( char* file, char* destdir, const char** patterns, uint numPatterns )
{
	FILE*	   f = NULL;
	HeaderType headerType;
	PakInfo*   info = NULL;
	Result	   result = ERR_OTHER;

	log( "Opening file: %s\n", file );
	f = fopen( file, "rb" );
	if( !f ) {
		perror( file );
		return ERR_IO;
	}
	headerType = detect_header( f );
	if( headerType == HEADER_UNKNOWN ) {
		fprintf_s( stderr, "Unrecognized file format\n" );
		fclose( f );
		return ERR_FILE_FORMAT;
	} else if( headerType == HEADER_INSTALL ) {
		// If it's an installer pak, we need to start by the first volume
		char* ext = extension( file );
		if( strcmp( ext, ".001" ) ) {
			fclose( f );
			return process_first_volume( file, ext, destdir, patterns, numPatterns );
		}
	}
	result = read_pak_info( f, headerType, &info, patterns, numPatterns );
	if( !list && result == ERR_OK ) {
		if( destdir ) {
			result = check_destdir( destdir );
			if( result != ERR_OK ) {
				fclose( f );
				free_PakInfo( info );
				if( result == ERR_IO ) {
					perror( destdir );
				} else {   // result == ERR_COMMAND_LINE
					fprintf_s( stderr, "Not a valid directory: %." S_MAX_PATH "s\n", destdir );
				}
				return result;
			}
		}
		result = extract( f, file, info, destdir );
	}
	fclose( f );
	free_PakInfo( info );
	return result;
}

int main( int argc, char* argv[] )
{
	bool		 noMoreOptions = false, readDestDir = false;
	char *		 file = NULL, *destdir = NULL;
	const char** patterns = NULL;
	uint		 numPatterns = 0;
	int			 i;
	char*		 p;
	Result		 result;

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
					case 'd':
						if( destdir ) {
							help();
							return ERR_COMMAND_LINE;
						}
						readDestDir = true;
						break;
					case 's': createDirs = true; break;
					case 'u': noLower = true; break;
					case 'k': keepGoing = true; break;
					case 'b': keepBroken = true; break;
					case 'v': verbose = 1; break;
					case 'h':
					case '?': help(); return ERR_OK;
					default: help(); return ERR_COMMAND_LINE;
				}
			}
		} else {
			if( readDestDir ) {
				readDestDir = false;
				destdir = argv[i];
			} else if( !file ) {
				file = argv[i];
			} else {
				if( !patterns ) {
					patterns = (const char**)malloc( sizeof( char* ) * ( argc - i ) );
					if( !patterns ) {
						perror( "Fatal" );
						return ERR_MEMORY;
					}
				}
				patterns[numPatterns++] = argv[i];
			}
		}
	}

	if( !file || ( list && destdir ) ) {
		help();
		return ERR_COMMAND_LINE;
	}

	result = process_file( file, destdir, patterns, numPatterns );
	if( patterns ) free( patterns );
	return result;
}
