#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <string.h>
#include <float.h>

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

Result read_pak_info( FILE* f, HeaderType type, PakInfo** pInfo )
{
	Result	  result = ERR_OTHER;
	PakInfo*  info = (PakInfo*)malloc( sizeof( PakInfo ) );
	uint	  i, totalZSize = 0, totalUSize = 0;
	FileInfo* fi;
	float	  ratio;

	assert( f );
	assert( pInfo );

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
		if( fread( &fi->name, 16, 1, f ) != 1 ) err( ERR_IO );
		if( strnlen_s( fi->name, 16 ) == 16 ) {
			if( keepGoing ) {
				fprintf_s( stderr, "WARNING: File #%u (%.16s): name with no zero terminator\n", i, fi->name );
				errors++;
			} else {
				fprintf_s( stderr, "ERROR: File #%u (%.16s): name with no zero terminator\n", i, fi->name );
				fprintf_s( stderr, MSG_TOIGNORE );
				err( ERR_FILE_FORMAT );
			}
		}
		if( !noLower ) {
			strlwr_s( fi->name, 16 );
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
			log( "%*u: %-16.16s offset: %-10u uSize: %-10u zSize: %-10u ratio: %3.1f%%\n", ndigits, i, fi->name,
			  fi->offset, fi->uSize, fi->zSize, ratio );
		} else if( list ) {
			printf_s( " %10u %10u %5.1f%% %-.16s\n", fi->uSize, fi->zSize, ratio, fi->name );
		}
	}
	// Print totals
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
	free_PakInfo( info );
	if( result == ERR_MEMORY || result == ERR_IO ) {
		perror( "Fatal" );
	}
	return result;
}

#undef err
