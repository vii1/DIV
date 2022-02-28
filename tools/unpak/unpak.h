#ifndef __UNPAK_H__
#define __UNPAK_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#if defined( __UNIX__ ) || defined( __LINUX__ ) || defined( __linux__ )
#	define SEP '/'
#else
#	define SEP '\\'
#endif

#define MSG_TOIGNORE "To ignore this error, use -k option\n"

#define __stringify( x ) #x
#define stringify( x ) __stringify( x )

// For some reason this is the max filename length in DIV PAK files
#define MAX_FILE 16
#define S_MAX_FILE stringify( MAX_FILE )

#define S_MAX_PATH stringify( _MAX_PATH )

typedef unsigned char byte;
typedef unsigned int  uint;

// Result codes
typedef enum {
	ERR_OK = 0,			// Successful
	ERR_COMMAND_LINE,	// Error in command line
	ERR_IO,				// I/O error or file not found
	ERR_FILE_FORMAT,	// Invalid file format or data
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
	char   name[MAX_FILE];	 // File name
	size_t offset;			 // File data offset in packfile
	size_t zSize;			 // Compressed file size (if ==uSize, it's not compressed)
	size_t uSize;			 // Uncompressed file size
} FileInfo;

// Installer only info
typedef struct {
	bool isFinalVolume;	  // Is this the last volume?
} InstallInfo;

// Game only info
typedef struct {
	byte version;	// File format version
	uint crc[3];	// Checksum (not true CRC) for compiled exes: 0=game exe, 1=setup.exe, 2=reserved
} GameInfo;

// Pak info struct
typedef struct {
	HeaderType type;
	union {
		InstallInfo install;
		GameInfo	game;
	};
	uint	  numFiles;	  // Number of files in packfile
	FileInfo* files;	  // File info data
} PakInfo;

extern int	verbose;
extern bool list;
extern bool keepGoing;
extern bool keepBroken;
extern bool createDirs;
extern bool noLower;
extern uint errors;

char*	   strlwr_s( char* str, size_t maxsize );
void	   log( const char* fmt, ... );
char*	   extension( char* path );
Result	   check_destdir( char* destdir );
void	   set_volume_extension( char* filename, uint volume );
HeaderType detect_header( FILE* f );

void   free_PakInfo( PakInfo* p );
Result read_pak_info( FILE* f, HeaderType type, PakInfo** pInfo, const char** patterns, uint numPatterns );

Result extract( FILE* f, char* filename, const PakInfo* info, const char* destdir );

#endif	 // __UNPAK_H__
