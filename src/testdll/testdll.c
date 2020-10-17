#include <stdio.h>
#include <unistd.h>
#include "divdll.h"

static int fake_mem[10];

int execute_divmain = 0, execute_divlibrary = 0, execute_divend = 0;
int report_exports = 0, report_imports = 0;
int verbosity = 0;
int dump_files = 0;

void HYB_export( char* name, void* dir, int nparms );

void fake_func()
{
}

void* Proxy_DIV_import( char* name )
{
	if( report_imports ) {
		printf( "DLL imports: %s\n", name );
	}
	return DIV_import( name );
}

void Proxy_DIV_export( char* name, void* obj )
{
	if( report_exports ) {
		printf( "DLL exports: %s @ 0x%lx\n", name, obj );
	}
	DIV_export( name, obj );
}

void Proxy_HYB_export( char* name, void* dir, int nparms )
{
	if( report_exports ) {
		printf( "DLL exports function %s with %d parameters @ 0x%lx\n", name, nparms, dir );
	}
	HYB_export( name, dir, nparms );
}

PE* DIV_LoadDll( const char* name )
{
	PE* pefile;
	void ( *entryp )( void* ( *DIV_import )(), void ( *DIV_export )() );
	void ( *entryp2 )( void ( *HYB_export )() );

	// reset error condition

	dll_error = NULL;

	// try to read the file (portable executable format)

	if( ( pefile = PE_ReadFN( name ) ) == NULL ) return NULL;

	// find the entrypoint

	entryp = PE_ImportFnc( pefile, "divmain_" );
	if( entryp == NULL ) {
		entryp = PE_ImportFnc( pefile, "_divmain" );
	}
	if( entryp == NULL ) {
		entryp = PE_ImportFnc( pefile, "W?divmain" );
	}

	if( entryp == NULL ) {
		PE_Free( pefile );
		dll_error = "Couldn't find DIV entrypoint";
		return NULL;
	}
	// execute entrypoint
	if( execute_divmain ) {
		entryp( Proxy_DIV_import, Proxy_DIV_export );
	}

	// check if entrypoint was successfull
	if( dll_error != NULL ) {
		// no ? free pefile and return NULL;
		PE_Free( pefile );
		return NULL;
	}

	entryp2 = PE_ImportFnc( pefile, "divlibrary_" );
	if( entryp2 == NULL ) {
		entryp2 = PE_ImportFnc( pefile, "_divlibrary" );
	}
	if( entryp2 == NULL ) {
		entryp2 = PE_ImportFnc( pefile, "W?divlibrary" );
	}

	if( entryp2 == NULL ) {
		PE_Free( pefile );
		dll_error = "Couldn't find DIV divlibrary";
		return NULL;
	}
	// execute entrypoint
	if( execute_divlibrary ) {
		entryp2( Proxy_HYB_export );
	}
	// check if entrypoint was successfull
	if( dll_error != NULL ) {
		// no ? free pefile and return NULL;
		PE_Free( pefile );
		return NULL;
	}

	return pefile;
}

void DIV_UnLoadDll( PE* pefile )
{
	void ( *entryp )( void* ( *DIV_import )(), void ( *DIV_export )() );

	// find the entrypoint (again)

	entryp = PE_ImportFnc( pefile, "divend_" );
	if( entryp == NULL ) {
		entryp = PE_ImportFnc( pefile, "_divend" );
	}
	if( entryp == NULL ) {
		entryp = PE_ImportFnc( pefile, "W?divend" );
	}
	if( entryp != NULL && execute_divend ) {
		entryp( DIV_import, DIV_RemoveExport );
	}
	PE_Free( pefile );
}

void show_help()
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
	PE* pe;
	int c;
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

	if(optind >=argc) {
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

	pe = DIV_LoadDll( fichero );
	if( !pe ) {
		fprintf( stderr, "Error: %s\n", dll_error );
		return 1;
	}

	DIV_UnLoadDll( pe );

	return 0;
}
