#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <string.h>
#include <float.h>

#include "unpak.h"

void free_InstallPakInfo( InstallPakInfo* p )
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

Result read_install_info( FILE* f, InstallPakInfo** pInfo )
{
	Result			result = ERR_OTHER;
	InstallPakInfo* info = (InstallPakInfo*)malloc( sizeof( InstallPakInfo ) );
	uint			i, totalZSize = 0, totalUSize = 0;
	FileInfo*		fi;
	float			ratio;

	assert( f );
	assert( pInfo );

	if( !info ) err( ERR_MEMORY );
	info->files = NULL;
	if( fread( &info->isFinalVolume, 1, 1, f ) != 1 ) err( ERR_IO );
	info->isFinalVolume = !info->isFinalVolume;
	log( "File is final volume: %s\n", info->isFinalVolume ? "YES" : "NO" );

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
			log( "%*u: %-16.16s offset: %-10u uSize: %-10u zSize: %-10u ratio: %3.1f%%\n", ndigits, i, fi->name,
			  fi->offset, fi->uSize, fi->zSize, ratio );
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
