#include <stdio.h>
#include "divdll.h"

static int fake_mem[10];

void HYB_export( char* name, void* dir, int nparms );

void fake_func()
{
}

void *Proxy_DIV_import(char *name) {
	//printf("DLL imports: %s\n", name);
	return DIV_import(name);
}

void Proxy_DIV_export(char *name,void *obj) {
	printf("DLL exports: %s\n", name);
	DIV_export(name, obj);
}

void Proxy_HYB_export(char *name,void *dir,int nparms){
	printf("DLL exports function %s with %d parameters\n", name, nparms);
	HYB_export(name, dir, nparms);
}

PE* DIV_LoadDll( char* name )
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
	entryp( Proxy_DIV_import, Proxy_DIV_export );

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
	entryp2( Proxy_HYB_export );
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
	if( entryp != NULL ) {
		entryp( DIV_import, DIV_RemoveExport );
	}
	PE_Free( pefile );
}

int main( int argc, char* argv[] )
{
	PE* pe;

	if( argc < 2 ) {
		printf( "Uso: testdll <archivo.dll>\n" );
		return 1;
	}

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

	pe = DIV_LoadDll( argv[1] );
	if( !pe ) {
		fprintf( stderr, "Error: %s\n", dll_error );
		return 1;
	}

	DIV_UnLoadDll( pe );

	return 0;
}
