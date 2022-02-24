#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "divdll.h"
#include "pe.h"

static int fake_mem[10];

int execute_divmain = 0, execute_divlibrary = 0, execute_divend = 0;
int report_exports = 0, report_imports = 0;
int verbosity = 0;
int dump_files = 0;

static void fake_func()
{
}

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

	printf( "Dumping section %s to file %s\n", h->Name, filename );

	if( ( f = fopen( filename, "wb" ) ) == NULL ) {
		fprintf( stderr, "Can't write file: %s\n", filename );
		dll_error = "Error opening file";
		return 0;
	}
	fwrite( h->PhysicalAddress, h->VirtualSize, 1, f );
	fclose( f );

	return 1;
}

FILE* open_file( const char* file )
{
	return fopen( file, "rb" );
}

static void* Proxy_DIV_import( const char* name )
{
	if( report_imports ) {
		printf( "DLL imports: %s\n", name );
	}
	return DIV_import( name );
}

static void Proxy_DIV_export( const char* name, void* obj )
{
	if( report_exports ) {
		printf( "DLL exports: %s @ 0x%lx\n", name, obj );
	}
	DIV_export( name, obj );
}

static void Proxy_DIV_RemoveExport( const char* name, void* obj )
{
	if( report_exports ) {
		printf( "DLL removes export: %s @ 0x%lx\n", name, obj );
	}
	DIV_RemoveExport( name, obj );
}

static void Proxy_COM_export( const char* name, void* dir, int nparms )
{
	if( report_exports ) {
		printf( "DLL exports function %s with %d parameters @ 0x%lx\n", name, nparms, dir );
	}
	// COM_export( name, dir, nparms );
}

static PE* LoadDll( const char* name )
{
	PE*			 pefile;
	divlibrary_t ep_divlibrary;
	divmain_t	 ep_divmain;

	// reset error condition
	dll_error = NULL;

	// try to read the file (portable executable format)
	pefile = PE_ReadFN( name );
	if( pefile == NULL ) return NULL;

	// find the entrypoint
	ep_divmain = PE_ImportFnc( pefile, "divmain_" );
	if( ep_divmain == NULL ) {
		ep_divmain = PE_ImportFnc( pefile, "_divmain" );
		if( ep_divmain == NULL ) {
			ep_divmain = PE_ImportFnc( pefile, "W?divmain" );
		}
	}
	if( ep_divmain == NULL ) {
		printf( "divmain: NOT found\n" );
	} else {
		printf( "divmain: FOUND @ %p\n", ep_divmain );
		// execute entrypoint
		if( execute_divmain ) {
			ep_divmain( Proxy_DIV_import, Proxy_DIV_export );
			// check if entrypoint was successfull
			if( dll_error != NULL ) {
				// no ? free pefile and return NULL;
				PE_Free( pefile );
				return NULL;
			}
		}
	}

	ep_divlibrary = PE_ImportFnc( pefile, "divlibrary_" );
	if( ep_divlibrary == NULL ) {
		ep_divlibrary = PE_ImportFnc( pefile, "_divlibrary" );
		if( ep_divlibrary == NULL ) {
			ep_divlibrary = PE_ImportFnc( pefile, "W?divlibrary" );
		}
	}
	// execute entrypoint
	if( ep_divlibrary == NULL ) {
		printf( "divlibrary: NOT found\n" );
	} else {
		printf( "divlibrary: FOUND @ %p\n", ep_divlibrary );
		if( execute_divlibrary ) {
			ep_divlibrary( Proxy_COM_export );
			// check if entrypoint was successfull
			if( dll_error != NULL ) {
				// no ? free pefile and return NULL;
				PE_Free( pefile );
				return NULL;
			}
		}
	}
	return pefile;
}

static void UnLoadDll( PE* pefile )
{
	divend_t ep_divend;

	// find the `divend` function

	ep_divend = PE_ImportFnc( pefile, "divend_" );
	if( ep_divend == NULL ) {
		ep_divend = PE_ImportFnc( pefile, "_divend" );
		if( ep_divend == NULL ) {
			ep_divend = PE_ImportFnc( pefile, "W?divend" );
		}
	}
	if( ep_divend ) {
		printf( "divend: FOUND @ %p\n", ep_divend );
		if( execute_divend ) {
			ep_divend( Proxy_DIV_import, Proxy_DIV_RemoveExport );
		}
	} else {
		printf( "divend: NOT found\n" );
	}
	PE_Free( pefile );
}

static void show_help()
{
	printf( "Use: testdll [options] <file.dll>\n"
			"Options:\n"
			"  -x{mlea}   Execute DLL code\n"
			"             You can add one or more of the following:\n"
			"               m = Execute divmain()\n"
			"               l = Execute divlibrary()\n"
			"               e = Execute divend()\n"
			"               a = Execute all\n"
			"  -E         Report DLL exports (requires -xm and/or -xl)\n"
			"  -I         Report DLL imports (requires -xm)\n"
			"  -d         Dump sections before and after relocation\n"
			"  -v         Increase verbosity\n"
			"  -h         Show this help\n" );
}

int main( int argc, char* argv[] )
{
	PE*			pe;
	int			c;
	const char* fichero;

	if( argc < 2 ) {
		show_help();
		return 1;
	}

	while( ( c = getopt( argc, argv, ":x:EIdvh" ) ) != -1 ) {
		switch( c ) {
			case 'x': {
				char* p = optarg;
				while( *p ) {
					switch( *p ) {
						case 'm': execute_divmain = 1; break;
						case 'l': execute_divlibrary = 1; break;
						case 'e': execute_divend = 1; break;
						case 'a':
							execute_divmain = 1;
							execute_divlibrary = 1;
							execute_divend = 1;
							break;
						default: show_help(); return 1;
					}
					++p;
				}
			} break;
			case 'E': report_exports = 1; break;
			case 'I': report_imports = 1; break;
			case 'd': dump_files = 1; break;
			case 'v': ++verbosity; break;
			case ':': break;
			default: show_help(); return 1;
		}
	}

	if( optind >= argc ) {
		show_help();
		return 1;
	}
	fichero = argv[optind];

	DIV_export( "div_fopen", (void*)fake_func );
	DIV_export( "div_fclose", (void*)fake_func );
	DIV_export( "div_malloc", (void*)fake_func );
	DIV_export( "div_free", (void*)fake_func );
	DIV_export( "div_rand", (void*)fake_func );
	DIV_export( "div_text_out", (void*)fake_func );

	DIV_export( "stack", (void*)fake_mem );
	DIV_export( "sp", (void*)fake_mem );
	DIV_export( "wide", (void*)fake_mem );
	DIV_export( "height", (void*)fake_mem );
	DIV_export( "buffer", (void*)fake_mem );
	DIV_export( "background", (void*)fake_mem );
	DIV_export( "ss_time", (void*)fake_mem );
	DIV_export( "ss_status", (void*)fake_mem );
	DIV_export( "ss_exit", (void*)fake_mem );

	DIV_export( "process_size", (void*)fake_mem );
	DIV_export( "id_offset", (void*)fake_mem );
	DIV_export( "id_init_offset", (void*)fake_mem );
	DIV_export( "id_start_offset", (void*)fake_mem );
	DIV_export( "id_end_offset", (void*)fake_mem );
	DIV_export( "set_palette", (void*)fake_mem );
	DIV_export( "ghost", (void*)fake_mem );
	DIV_export( "region", (void*)fake_mem );

	DIV_export( "mem", (void*)fake_mem );
	DIV_export( "palette", (void*)fake_mem );
	DIV_export( "active_palette", (void*)fake_mem );
	DIV_export( "key", (void*)fake_mem );

	pe = LoadDll( fichero );
	if( !pe ) {
		fprintf( stderr, "Error: %s\n", dll_error );
		return 1;
	}

	UnLoadDll( pe );

	return 0;
}
