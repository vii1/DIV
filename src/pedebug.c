#ifdef PEDEBUG

#include "testdll/pe.h"

int verbosity = 1;
int dump_files = 1;

static void make_dos_filename( const char* str, char* dst )
{
	static const char allowed[] = "_^$~!#%&-{}@`'()";

	const char* p = str;
	int			i = 0;

	while( *p && i < 8 ) {
		if( ( *p >= 'A' && *p <= 'Z' ) || ( *p >= 'a' && *p <= 'z' ) || ( *p >= '0' && *p <= '9' ) ) {
			dst[i] = *p;
		} else {
			int x = sizeof( allowed );
			dst[i] = '_';
			for( ; x >= 0; --x ) {
				if( *p == allowed[x] ) {
					dst[i] = *p;
					break;
				}
			}
		}
		++p;
		++i;
	}
	dst[i] = 0;
}

int dump_section( const IMAGE_SECTION_HEADER* h, const char* ext )
{
	char  filename[13];
	FILE* f;

	if( h->SizeOfRawData == 0 || ( h->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA ) ) return 1;

	make_dos_filename( h->Name, filename );

	strcat( filename, "." );
	strncat( filename, ext, 3 );

	//printf( "Dumping section %s to file %s\n", h->Name, filename );

	if( ( f = fopen( filename, "wb" ) ) == NULL ) {
		fprintf( stderr, "Can't write file: %s\n", filename );
		dll_error = "Error opening file";
		return 0;
	}
	fwrite( h->PhysicalAddress, h->VirtualSize, 1, f );
	fclose( f );

	return 1;
}

#endif // PEDEBUG
