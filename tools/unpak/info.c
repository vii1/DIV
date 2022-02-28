#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <string.h>
#include <float.h>
#include <fnmatch.h>

#include "unpak.h"

void free_PakInfo( PakInfo* p )
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

Result read_pak_info( FILE* f, HeaderType type, PakInfo** pInfo, const char** patterns, uint numPatterns )
{
	Result	  result = ERR_OTHER;
	PakInfo*  info = (PakInfo*)malloc( sizeof( PakInfo ) );
	uint	  i, totalZSize = 0, totalUSize = 0, excluded = 0;
	FileInfo* fi;
	float	  ratio;

	assert( f );
	assert( pInfo );
	assert( numPatterns == 0 || patterns );

	if( !info ) err( ERR_MEMORY );
	info->type = type;
	info->files = NULL;
	if( type == HEADER_GAME ) {
		// Version must be 0
		if( fread( &info->game.version, 1, 1, f ) != 1 ) err( ERR_IO );
		log( "File version: %u\n", info->game.version );
		if( info->game.version != 0 ) {
			if( keepGoing ) {
				fprintf_s( stderr, "WARNING: File version is %u (expected 0), probably unsupported\n", info->game.version );
				errors++;
			} else {
				fprintf_s( stderr, "ERROR: File version is %u (expected 0)\n", info->game.version );
				fprintf_s( stderr, MSG_TOIGNORE );
				err( ERR_VERSION );
			}
		}
		// Checksums for exes
		for( i = 0; i < 3; ++i ) {
			if( fread( &info->game.crc[i], 4, 1, f ) != 1 ) err( ERR_IO );
			log( "Checksum #%u: 0x%08X\n", i, info->game.crc[i] );
		}
	} else {   // type == HEADER_INSTALL
		byte notFinalVol;
		if( fread( &notFinalVol, 1, 1, f ) != 1 ) err( ERR_IO );
		info->install.isFinalVolume = !notFinalVol;
		log( "File is final volume: %s\n", info->install.isFinalVolume ? "YES" : "NO" );
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
		// Read file name
		if( fread( &fi->name, MAX_FILE, 1, f ) != 1 ) err( ERR_IO );
		if( strnlen_s( fi->name, MAX_FILE ) == MAX_FILE ) {
			if( keepGoing ) {
				fprintf_s( stderr, "WARNING: File #%u (%." S_MAX_FILE "s): name with no zero terminator\n", i, fi->name );
				errors++;
			} else {
				fprintf_s( stderr, "ERROR: File #%u (%." S_MAX_FILE "s): name with no zero terminator\n", i, fi->name );
				fprintf_s( stderr, MSG_TOIGNORE );
				err( ERR_FILE_FORMAT );
			}
		}
		// Filter by pattern, if any
		if( numPatterns > 0 ) {
			bool match = false;
			uint j;
			for( j = 0; j < numPatterns; ++j ) {
				if( fnmatch( patterns[j], fi->name, FNM_PATHNAME | FNM_NOESCAPE | FNM_IGNORECASE ) == 0 ) {
					match = true;
					break;
				}
			}
			if( !match ) {
				++excluded;
				--fi;
				fseek( f, 12, SEEK_CUR );
				continue;
			}
		}
		if( !noLower ) {
			strlwr_s( fi->name, MAX_FILE );
		}
		// Read sizes
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
		// Print out if desired
		if( verbose ) {
			int	 ndigits = 0;
			uint n = info->numFiles;
			while( n ) {
				++ndigits;
				n /= 10;
			}
			log( "%*u: %-16." S_MAX_FILE "s offset: %-10u uSize: %-10u zSize: %-10u ratio: %3.1f%%\n", ndigits, i,
			  fi->name, fi->offset, fi->uSize, fi->zSize, ratio );
		} else if( list ) {
			printf_s( " %10u %10u %5.1f%% %-." S_MAX_FILE "s\n", fi->uSize, fi->zSize, ratio, fi->name );
		}
	}
	info->numFiles -= excluded;
	// Print totals
	if( totalUSize == 0 ) {
		ratio = _INFF;
	} else {
		ratio = ( (float)totalZSize / totalUSize ) * 100.f;
	}
	if( verbose ) {
		log( "TOTAL: uSize: %-10u zSize: %-10u ratio: %.1f%%\n", totalUSize, totalZSize, ratio );
		if( excluded > 0 ) {
			log( "%u file(s) excluded\n", excluded );
		}
	}
	if( list && !verbose ) {
		printf_s( "----------- ---------- ------ ----------------\n"
				  " %10u %10u %5.1f%% %u file(s) total",
		  totalUSize, totalZSize, ratio, info->numFiles );
		if( excluded > 0 ) {
			printf_s( " (%u excluded)\n", excluded );
		} else {
			printf_s( "\n" );
		}
	}
	*pInfo = info;
	return ERR_OK;

_error:
	*pInfo = NULL;
	free_PakInfo( info );
	if( result == ERR_MEMORY || result == ERR_IO ) {
		perror( "Fatal" );
	}
	return result;
}

#undef err
