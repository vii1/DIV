#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <string.h>
#include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include "zlib.h"
#include "unpak.h"

#define CHUNK ( 128 * 1024 )

unsigned char buffer_in[CHUNK];
unsigned char buffer_out[CHUNK];

typedef struct {
	z_stream strm;
	bool	 should_deinit;
} ZfcopyStatus;

void try_remove( const char* file )
{
	int errno_save = errno;
	if(keepBroken) {
		fprintf_s(stderr, "WARNING: Keeping partially extracted file %s\n", file);
	} else if( remove( file ) ) {
		fprintf_s( stderr, "%s: ", file );
		perror( "Error deleting partial file" );
	}
	errno = errno_save;
}

// Builds the path in which a file will be written.
// Depending on the conditions:
// - destdir is specified?
// - createDirs is specified?
// - is the extension EXE, DLL or PAK?
// the following patterns can be generated:
// - filename
// - extension\filename
// - destdir\filename
// - destdir\extension\filename
// TODO: check for path too long
// TODO: Make resulting path static?
char* get_destfile( const char* destdir, char* file )
{
	size_t destdir_len = destdir ? strnlen_s( destdir, _MAX_PATH ) : 0;
	size_t file_len = strnlen_s( file, MAX_FILE );
	size_t path_len;
	char*  path;
	if( createDirs ) {
		char* ext = extension( file );
		if( *ext == '.' ) ++ext;
		if( *ext != 0 && strnicmp( ext, "exe", 4 ) && strnicmp( ext, "dll", 4 ) && strnicmp( ext, "pak", 4 ) ) {
			size_t ext_len = strnlen_s( ext, file_len );
			size_t len;
			Result result;
			if( destdir ) {
				path_len = destdir_len + ext_len + file_len + 2;
			} else {
				path_len = ext_len + file_len + 1;
			}
			path = (char*)malloc( path_len + 1 );
			if( path == NULL ) return NULL;
			if( destdir ) {
				snprintf_s( path, path_len + 1, "%s%c%s", destdir, SEP, ext );
				len = destdir_len + ext_len + 1;
			} else {
				strncpy_s( path, path_len + 1, ext, ext_len );
				len = ext_len;
			}
			result = check_destdir( path );
			if( result == ERR_IO ) {
				perror( path );
				goto _error;
			} else if( result == ERR_COMMAND_LINE ) {
				fprintf_s( stderr, "Not a valid directory: %." S_MAX_PATH "s\n", path );
				goto _error;
			}
			path[len] = SEP;
			strncpy_s( path + len + 1, path_len + 1, file, file_len );
			return path;
		}
	}
	if( !destdir ) return strdup( file );
	path_len = destdir_len + file_len + 1;
	path = (char*)malloc( path_len + 1 );
	if( path == NULL ) return NULL;
	snprintf_s( path, path_len + 1, "%s%c%s", destdir, SEP, file );
	return path;
_error:
	free( path );
	return NULL;
}

// Reads a number of bytes from a stream and writes them to a file
// Returns ERR_OK on success, ERR_IO on error
Result fcopy( FILE* from, size_t size, const char* destfile, bool append )
{
	size_t i, n = size % CHUNK;
	int	   hto, hfrom = fileno( from );	  // Bypass stream buffering - CHECK!
	if( hfrom == -1 || fflush( from ) ) return ERR_IO;
	if( append ) {
		hto = open( destfile, _O_WRONLY | _O_BINARY | _O_APPEND );
	} else {
		hto = open( destfile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666 );
	}
	if( hto == -1 ) return ERR_IO;
	for( i = 0; i < size / CHUNK; ++i ) {
		if( read( hfrom, buffer_in, CHUNK ) != CHUNK ) goto _error;
		if( write( hto, buffer_in, CHUNK ) != CHUNK ) goto _error;
	}
	if( n ) {
		if( read( hfrom, buffer_in, n ) != n ) goto _error;
		if( write( hto, buffer_in, n ) != n ) goto _error;
	}
	close( hto );
	return ERR_OK;

_error : {
	int temp_errno = errno;
	close( hto );
	// try_remove( destfile );
	errno = temp_errno;
	return ERR_IO;
}
}

// Converts a ZLib result code to one of my result codes
Result zresult( int ret )
{
	switch( ret ) {
		case Z_OK:
		case Z_STREAM_END: return ERR_OK;
		case Z_MEM_ERROR: return ERR_MEMORY;
		case Z_DATA_ERROR:
		case Z_STREAM_ERROR: return ERR_FILE_FORMAT;
		case Z_VERSION_ERROR: return ERR_VERSION;
		case Z_ERRNO: return ERR_IO;
		default: return ERR_OTHER;
	}
}

// Simplest error handling
#define err( x )     \
	{                \
		result = x;  \
		goto _error; \
	}

// https://www.zlib.net/zlib_how.html
/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
Result zfcopy( FILE* from, size_t size, const char* destfile, ZfcopyStatus* status, bool append )
{
	int	   ret;
	Result result = ERR_OK;
	// bool   deinit_inflate = false;
	size_t bytes_read = 0;

	int hto, hfrom = fileno( from );   // Bypass stream buffering - CHECK!
	if( hfrom == -1 || fflush( from ) ) return ERR_IO;
	if( append ) {
		hto = open( destfile, _O_WRONLY | _O_BINARY | _O_APPEND );
	} else {
		status->should_deinit = false;
		hto = open( destfile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666 );
	}
	if( hto == -1 ) return ERR_IO;

	if( !append ) {
		/* allocate inflate state */
		status->strm.zalloc = Z_NULL;
		status->strm.zfree = Z_NULL;
		status->strm.opaque = Z_NULL;
		status->strm.avail_in = 0;
		status->strm.next_in = Z_NULL;
		ret = inflateInit( &status->strm );
		if( ret != Z_OK ) err( zresult( ret ) );
		status->should_deinit = true;
	}
	/* decompress until deflate stream ends or end of file */
	while( bytes_read < size && ret != Z_STREAM_END ) {
		int n = read( hfrom, buffer_in, min( size - bytes_read, CHUNK ) );
		if( n < 0 ) err( ERR_IO );
		if( n == 0 ) break;
		bytes_read += n;
		status->strm.avail_in = (uint)n;
		status->strm.next_in = buffer_in;
		/* run inflate() on input until output buffer not full */
		do {
			unsigned have;
			// printf( "avail_in: %u\n", status->strm.avail_in );
			status->strm.avail_out = CHUNK;
			status->strm.next_out = buffer_out;
			ret = inflate( &status->strm, Z_NO_FLUSH );
			assert( ret != Z_STREAM_ERROR ); /* state not clobbered */
			switch( ret ) {
				case Z_NEED_DICT: ret = Z_DATA_ERROR; /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR: err( zresult( ret ) );
			}
			have = CHUNK - status->strm.avail_out;
			n = write( hto, buffer_out, have );
			if( n < 0 || n != (int)have ) err( ERR_IO );
			// printf( "have: %u, avail_out: %u\n", have, status->strm.avail_out );
		} while( status->strm.avail_out == 0 );
		/* done when inflate() says it's done */
	}
	/* clean up and return */
	close( hto );
	// inflateEnd( &status->strm );
	return zresult( ret );

_error : {
	int temp_errno = errno;
	close( hto );
	// try_remove( destfile );
	errno = temp_errno;
	return result;
}
}

// Open the next volume, check it's valid and update variables
Result open_volume( FILE* f, char* filename, uint volume, bool* pFinalVol, int* pVolOffset, size_t* pFileSize )
{
	// TODO: Ask for next disk?
	FILE*  newf = NULL;
	byte   notFinalVol;
	Result result = ERR_OTHER;
	set_volume_extension( filename, volume );
	log( "Opening file: %s\n", filename );
	if( freopen_s( &newf, filename, "rb", f ) != 0 || newf == NULL ) err( ERR_IO );
	if( detect_header( f ) != HEADER_INSTALL ) err( ERR_FILE_FORMAT );
	if( fread( &notFinalVol, 1, 1, f ) != 1 ) err( ERR_IO );
	*pFinalVol = !notFinalVol;
	log( "File is final volume: %s\n", !notFinalVol ? "YES" : "NO" );
	*pVolOffset += *pFileSize - 8;
	fseek( f, 0, SEEK_END );
	*pFileSize = ftell( f );
	return ERR_OK;

_error:
	if( result == ERR_IO ) {
		perror( filename );
	} else {   // result == ERR_FILE_FORMAT
		fprintf_s( stderr, "ERROR: %." S_MAX_PATH "s: Unrecognized file format\n", filename );
	}
	return result;
}

// Extracts the files from a pak
Result extract( FILE* f, char* filename, const PakInfo* info, const char* destdir )
{
	// Current volume number
	uint vol = 1;
	// volOffset allows us to calculate the physical offset in the volume where a file is stored.
	int volOffset = info->type == HEADER_INSTALL ? -8 : 0;
	// Current file index
	uint i = 0;
	// Current volume filesize
	size_t filesize;
	// Is the current volume final?
	bool   finalVol = info->type == HEADER_GAME || info->install.isFinalVolume;
	Result result = ERR_OK;

	fseek( f, 0, SEEK_END );
	filesize = ftell( f );

	for( ; i < info->numFiles; ++i ) {
		FileInfo*	 fi = &info->files[i];
		char*		 destfile;
		size_t		 localOffset = fi->offset - volOffset;
		size_t		 available, missing;
		ZfcopyStatus status;
		// TODO: Verify fi->offset >= volOffset
		while( localOffset >= filesize ) {
			if( finalVol ) {
				size_t maxOffset = volOffset + filesize;
				if( keepGoing ) {
					fprintf_s( stderr, "WARNING: %." S_MAX_FILE "s: invalid file offset (%u >= %u), skipping\n",
					  fi->name, fi->offset, maxOffset );
					errors++;
					goto _nextfile;	  // Continue outer loop
				} else {
					fprintf_s( stderr, "ERROR: %." S_MAX_FILE "s: invalid file offset (%u >= %u)\n", fi->name, fi->offset, maxOffset );
					fprintf_s( stderr, MSG_TOIGNORE );
					err( ERR_FILE_FORMAT );
				}
			}
			result = open_volume( f, filename, ++vol, &finalVol, &volOffset, &filesize );
			if( result != ERR_OK ) err( result );
			localOffset = fi->offset - volOffset;
		}
		fseek( f, localOffset, SEEK_SET );

		destfile = get_destfile( destdir, fi->name );
		if( destfile == NULL ) {
			if( keepGoing ) {
				fprintf_s( stderr, "WARNING: Skipping file %." S_MAX_FILE "s\n", fi->name );
				errors++;
				continue;
			} else {
				fprintf_s( stderr, MSG_TOIGNORE );
				err( ERR_OTHER );
			}
		}
		if( verbose ) {
			log( "Extracting: %." S_MAX_PATH "s\n", destfile );
		} else {
			printf_s( "Extracting: %." S_MAX_FILE "s\n", fi->name );
		}
		// How much of this file is in this volume?
		available = min( filesize - localOffset, fi->zSize );
		// Extract the first (or only) part
		status.should_deinit = false;
		if( fi->uSize == fi->zSize ) {
			// File is uncompressed
			result = fcopy( f, available, destfile, false );
		} else {
			// File is compressed
			result = zfcopy( f, available, destfile, &status, false );
		}
		if( result != ERR_OK ) goto _extract_error;
		missing = fi->zSize - available;
		while( missing > 0 ) {
			// File is incomplete in this volume
			if( finalVol ) {
				if( status.should_deinit ) inflateEnd( &status.strm );
				if( keepGoing ) {
					fprintf_s( stderr, "WARNING: %." S_MAX_FILE "s: file is incomplete (%u bytes missing)\n", fi->name, missing );
					errors++;
					try_remove( destfile );
					// All other files will report errors, but whatever
					break;	 // continue outer loop
				} else {
					fprintf_s( stderr, "ERROR: %." S_MAX_FILE "s: file is incomplete (%u bytes missing)\n", fi->name, missing );
					fprintf_s( stderr, MSG_TOIGNORE );
					if( status.should_deinit ) inflateEnd( &status.strm );
					try_remove( destfile );
					free( destfile );
					err( ERR_FILE_FORMAT );
				}
			}
			// Open next volume
			result = open_volume( f, filename, ++vol, &finalVol, &volOffset, &filesize );
			if( result != ERR_OK ) {
				if( status.should_deinit ) inflateEnd( &status.strm );
				try_remove( destfile );
				err( result );
			}
			localOffset = 8;
			fseek( f, localOffset, SEEK_SET );
			available = min( filesize - localOffset, missing );
			// Extract partial file
			if( fi->uSize == fi->zSize ) {
				result = fcopy( f, available, destfile, true );
			} else {
				result = zfcopy( f, available, destfile, &status, true );
			}
			if( result != ERR_OK ) goto _extract_error;
			missing -= available;
		}
	_nextfile:
		if( status.should_deinit ) inflateEnd( &status.strm );
		free( destfile );
		continue;

	_extract_error:
		if( status.should_deinit ) inflateEnd( &status.strm );
		if( keepGoing && result != ERR_MEMORY ) {
			if( result == ERR_IO ) {
				perror( destfile );
			} else {
				fprintf_s( stderr, "WARNING: %." S_MAX_FILE "s: invalid data\n", fi->name );
			}
			fprintf_s( stderr, "WARNING: Skipping file %." S_MAX_FILE "s\n", fi->name );
			try_remove( destfile );
			free( destfile );
			errors++;
			continue;
		} else {
			if( result == ERR_IO ) {
				perror( destfile );
			} else if( result == ERR_MEMORY ) {
				perror( "Fatal" );
			} else {
				fprintf_s( stderr, "ERROR: %." S_MAX_FILE "s: invalid data\n", fi->name );
			}
			try_remove( destfile );
			free( destfile );
			if( result != ERR_MEMORY ) {
				fprintf_s( stderr, MSG_TOIGNORE );
			}
			err( result );
		}
	}

_error:
	if( f ) fclose( f );
	// if( result == ERR_MEMORY || result == ERR_IO ) {
	// 	perror( "Fatal" );
	// }
	if( errors == 0 ) {
		printf_s( "%u file(s) extracted\n", i );
	} else {
		printf_s( "%u file(s) extracted. WARNING: %u failed\n", i - errors, errors );
	}
	return result;
}

#undef err
