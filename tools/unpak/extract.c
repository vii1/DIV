#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <string.h>

#include "unpak.h"

#define err( x )     \
	{                \
		result = x;  \
		goto _error; \
	}

Result fcopy(FILE* to, FILE* from, size_t size)
{
	byte buffer[BUFSIZ];
	size_t i;
	for(i=0; i<size / BUFSIZ; ++i) {
		if(fread(buffer, size, 1, from) != 1) goto _error;
		_
	}

_error:
	return ERR_OK;
}

Result extract( FILE* f, char* filename, const PakInfo* info, uint firstFile, const char* destdir )
{
	int	   volOffset = info->type == HEADER_INSTALL ? -8 : 0;
	uint   i = firstFile;
	size_t filesize;
	bool   finalVol = info->type == HEADER_GAME || info->install.isFinalVolume;
	Result result = ERR_OTHER;

	fseek( f, 0, SEEK_END );
	filesize = ftell( f );

	for( ; i < info->numFiles; ++i ) {
		FileInfo* fi = &info->files[i];
		if( fi->offset - volOffset >= filesize ) {
			if( finalVol ) {
				size_t maxOffset = volOffset + filesize;
				if( keepGoing ) {
					fprintf_s( stderr, "WARNING: %.16s: invalid file offset (%u >= %u), skipping\n", fi->name, fi->offset, maxOffset );
					errors++;
					continue;
				} else {
					fprintf_s( stderr, "ERROR: %.16s: invalid file offset (%u >= %u)\n", fi->name, fi->offset, maxOffset );
					fprintf_s( stderr, MSG_TOIGNORE );
					err( ERR_FILE_FORMAT );
				}
			}
			// TODO: next volume - freopen ?
		}
		fseek( f, fi->offset - volOffset, SEEK_SET );
		printf_s( "Extracting: %.16s\n", fi->name );
		if( fi->offset - volOffset + fi->zSize > filesize ) {
			// File is incomplete in this volume
			if( finalVol ) {
				size_t missing = fi->offset - volOffset + fi->zSize - filesize;
				if( keepGoing ) {
					fprintf_s( stderr, "WARNING: %.16s: file is incomplete (%u bytes missing), skipping\n", fi->name, missing );
					errors++;
					continue;
				} else {
					fprintf_s( stderr, "ERROR: %.16s: file is incomplete (%u bytes missing)\n", fi->name, missing );
					fprintf_s( stderr, MSG_TOIGNORE );
					err( ERR_FILE_FORMAT );
				}
			}
			// TODO: Extract partial file and continue in next volume
		} else if( fi->uSize == fi->zSize ) {
			// File is whole and uncompressed
			
		} else {
			// File is whole and compressed
		}
	}

_error:
	if( f ) fclose( f );
	if( result == ERR_MEMORY || result == ERR_IO ) {
		perror( "Fatal" );
	}
	return result;
}

#undef err
