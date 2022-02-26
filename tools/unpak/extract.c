#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <string.h>
#include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

#include "unpak.h"

#define CHUNK ( 128 * 1024 )

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
			size_t len = 0;
			Result result;
			if( destdir ) {
				path_len = destdir_len + ext_len + file_len + 2;
			} else {
				path_len = ext_len + file_len + 1;
			}
			path = (char*)malloc( path_len + 1 );
			if( path == NULL ) return NULL;
			if( destdir ) {
				if( memcpy_s( path, path_len, destdir, destdir_len ) ) goto _error;
				path[destdir_len] = SEP;
				len = destdir_len + 1;
			}
			if( memcpy_s( path + len, path_len - len, ext, ext_len ) ) goto _error;
			len += ext_len;
			path[len] = 0;
			result = check_destdir( path );
			if( result == ERR_IO ) {
				perror( path );
				goto _error;
			} else if( result == ERR_COMMAND_LINE ) {
				fprintf_s( stderr, "Not a valid directory: %." S_MAX_PATH "s\n", path );
				goto _error;
			}
			path[len] = SEP;
			if( memcpy_s( path + len + 1, path_len - len - 1, file, file_len ) ) goto _error;
			path[path_len] = 0;
			return path;
		}
	}
	if( !destdir ) return strdup( file );
	path_len = destdir_len + file_len + 1;
	path = (char*)malloc( path_len + 1 );
	if( path == NULL ) return NULL;
	if( memcpy_s( path, path_len, destdir, destdir_len ) ) goto _error;
	path[destdir_len] = SEP;
	if( memcpy_s( path + destdir_len + 1, path_len - destdir_len - 1, file, file_len ) ) goto _error;
	path[path_len] = 0;
	return path;
_error:
	free( path );
	return NULL;
}

// Reads a chunk from a stream and writes it to a file
// Returns ERR_OK on success, ERR_IO on error
Result fcopy( FILE* from, size_t size, const char* destfile )
{
	byte   buffer[CHUNK];
	size_t i, n = size % CHUNK;
	int	   hto, hfrom = fileno( from );	  // Bypass stream buffering - CHECK!
	if( hfrom == -1 || fflush( from ) ) return ERR_IO;
	hto = open( destfile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666 );
	if( hto == -1 ) return ERR_IO;
	for( i = 0; i < size / CHUNK; ++i ) {
		if( read( hfrom, buffer, CHUNK ) != CHUNK ) goto _error;
		if( write( hto, buffer, CHUNK ) != CHUNK ) goto _error;
	}
	if( n ) {
		if( read( hfrom, buffer, n ) != n ) goto _error;
		if( write( hto, buffer, n ) != n ) goto _error;
	}
	close( hto );
	return ERR_OK;

_error : {
	int temp_errno = errno;
	close( hto );
	if( remove( destfile ) ) perror( "Error deleting partial file" );
	errno = temp_errno;
	return ERR_IO;
}
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
// int inf( FILE* source, FILE* dest )
// {
// 	int			  ret;
// 	unsigned	  have;
// 	z_stream	  strm;
// 	unsigned char in[CHUNK];
// 	unsigned char out[CHUNK];
// 	/* allocate inflate state */
// 	strm.zalloc = Z_NULL;
// 	strm.zfree = Z_NULL;
// 	strm.opaque = Z_NULL;
// 	strm.avail_in = 0;
// 	strm.next_in = Z_NULL;
// 	ret = inflateInit( &strm );
// 	if( ret != Z_OK ) return ret;
// 	/* decompress until deflate stream ends or end of file */
// 	do {
// 		strm.avail_in = fread( in, 1, CHUNK, source );
// 		if( ferror( source ) ) {
// 			(void)inflateEnd( &strm );
// 			return Z_ERRNO;
// 		}
// 		if( strm.avail_in == 0 ) break;
// 		strm.next_in = in;
// 		/* run inflate() on input until output buffer not full */
// 		do {
// 			strm.avail_out = CHUNK;
// 			strm.next_out = out;
// 			ret = inflate( &strm, Z_NO_FLUSH );
// 			assert( ret != Z_STREAM_ERROR ); /* state not clobbered */
// 			switch( ret ) {
// 				case Z_NEED_DICT: ret = Z_DATA_ERROR; /* and fall through */
// 				case Z_DATA_ERROR:
// 				case Z_MEM_ERROR: (void)inflateEnd( &strm ); return ret;
// 			}
// 			have = CHUNK - strm.avail_out;
// 			if( fwrite( out, 1, have, dest ) != have || ferror( dest ) ) {
// 				(void)inflateEnd( &strm );
// 				return Z_ERRNO;
// 			}
// 		} while( strm.avail_out == 0 );
// 		/* done when inflate() says it's done */
// 	} while( ret != Z_STREAM_END );
// 	/* clean up and return */
// 	(void)inflateEnd( &strm );
// 	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
// }

#define err( x )     \
	{                \
		result = x;  \
		goto _error; \
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
		char*	  destfile;
		if( fi->offset - volOffset >= filesize ) {
			if( finalVol ) {
				size_t maxOffset = volOffset + filesize;
				if( keepGoing ) {
					fprintf_s( stderr, "WARNING: %." S_MAX_FILE "s: invalid file offset (%u >= %u), skipping\n",
					  fi->name, fi->offset, maxOffset );
					errors++;
					continue;
				} else {
					fprintf_s( stderr, "ERROR: %." S_MAX_FILE "s: invalid file offset (%u >= %u)\n", fi->name, fi->offset, maxOffset );
					fprintf_s( stderr, MSG_TOIGNORE );
					err( ERR_FILE_FORMAT );
				}
			}
			// TODO: next volume - freopen ?
			fprintf_s( stderr, "TODO: next volume\n" );
		}
		fseek( f, fi->offset - volOffset, SEEK_SET );

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
		if( fi->offset - volOffset + fi->zSize > filesize ) {
			// File is incomplete in this volume
			if( finalVol ) {
				size_t missing = fi->offset - volOffset + fi->zSize - filesize;
				free( destfile );
				if( keepGoing ) {
					fprintf_s( stderr, "WARNING: %." S_MAX_FILE "s: file is incomplete (%u bytes missing), skipping\n",
					  fi->name, missing );
					errors++;
					// All other files will report errors, but whatever
					continue;
				} else {
					fprintf_s( stderr, "ERROR: %." S_MAX_FILE "s: file is incomplete (%u bytes missing)\n", fi->name, missing );
					fprintf_s( stderr, MSG_TOIGNORE );
					err( ERR_FILE_FORMAT );
				}
			}
			// TODO: Extract partial file and continue in next volume
			fprintf_s( stderr, "TODO: Extract partial file and continue in next volume\n" );
		} else if( fi->uSize == fi->zSize ) {
			// File is whole and uncompressed
			result = fcopy( f, fi->uSize, destfile );
			if( result == ERR_IO ) {
				perror( destfile );
				free( destfile );
				if( keepGoing ) {
					fprintf_s( stderr, "WARNING: Skipping file %." S_MAX_FILE "s\n", fi->name );
					errors++;
					continue;
				} else {
					fprintf_s( stderr, MSG_TOIGNORE );
					err( ERR_IO );
				}
			}
		} else {
			// File is whole and compressed
			fprintf_s( stderr, "TODO: File is whole and compressed\n" );
		}
		free( destfile );
	}

_error:
	if( f ) fclose( f );
	if( result == ERR_MEMORY || result == ERR_IO ) {
		perror( "Fatal" );
	}
	return result;
}

#undef err
