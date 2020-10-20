/*
	MikDLL - Done by MikMak / HaRDCoDE '95
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIVDLL_C
#include "divdll.h"

#define UNUSED( x ) (void)x

#pragma pack( push, 1 )
typedef struct EXPORTENTRY {
	struct EXPORTENTRY* next;
	const char*			name;
	void*				obj;
} EXPORTENTRY;

static EXPORTENTRY* pool = NULL;
#pragma pack( pop )

#ifndef DIV32RUN
static char msgbuf[180];
#endif

COM_export_t COM_export;

static EXPORTENTRY* findexportentry( const char* name )
{
	EXPORTENTRY* e = pool;
	while( e != NULL ) {
		if( !strcmp( e->name, name ) ) break;
		e = e->next;
	}
	return e;
}

/*
	DIV_export
	===========

	Adds an object (function or a datablock) to the exports-pool so a DLL
	can use that object in the future.

	Input:
		char *name  :   The ASCIIZ name of the object to be exported, by
						which it will be registered in the exports-pool.
		void *obj   :   The address of the object to be exported.

*/
void DIV_export( const char* name, void* obj )
{
	EXPORTENTRY* e;

	// see if an export entry by this name already exists
	if( findexportentry( name ) != NULL ) return;

	e = malloc( sizeof( EXPORTENTRY ) );
	if( e ) {
		e->name = name;
		e->obj = obj;
		e->next = pool;
		pool = e;
	}
#ifdef PEDEBUG
	printf("DIV_export @ %p ( \"%s\", %p )\n", DIV_export, name, obj);
#endif
}

void DIV_RemoveExport( const char* name, void* obj )
{
	EXPORTENTRY* o;
	UNUSED( obj );
	o = findexportentry( name );
	if( o ) {
		EXPORTENTRY* e = pool;
		o->name = e->name;
		o->obj = e->obj;
		pool = e->next;
		free( e );
	}
}

/*
	DIV_import
	===========

	Finds the address of an object by reference of it's name. This routine
	is used by a DLL to access external functions/data.

	Input:
		char *name  :   The ASCIIZ name of the object to be imported.

	Returns:
		The address of the object or NULL when the object couldn't be found.
*/
void* DIV_import( const char* name )
{
	EXPORTENTRY* e;

	if( dll_error != NULL ) return NULL;

	e = findexportentry( name );
	if( !e ) {
#ifndef DIV32RUN
		sprintf( dll_error = msgbuf, "Couldn't resolve external reference: '%s'", name );
#endif
		return NULL;
	}
#ifdef PEDEBUG
	printf("DIV_import @ %p ( \"%s\" ) -> %p\n", DIV_import, name, e->obj);
#endif
	return e->obj;
}

/*
 * Descarga una DLL (int‚rprete)
 */
void DIV_UnLoadDll( PE* pefile )
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
		ep_divend( DIV_import, DIV_RemoveExport );
	}
	PE_Free( pefile );
}

static PE* Internal_LoadDll( const char* name, int execDivmain )
{
	PE*			 pefile;
	divlibrary_t ep_divlibrary;

	// reset error condition
	dll_error = NULL;

	// try to read the file (portable executable format)
	pefile = PE_ReadFN( name );
	if( pefile == NULL ) return NULL;

	if( execDivmain ) {
		divmain_t ep_divmain;
		// find the entrypoint
		ep_divmain = PE_ImportFnc( pefile, "divmain_" );
		if( ep_divmain == NULL ) {
			ep_divmain = PE_ImportFnc( pefile, "_divmain" );
			if( ep_divmain == NULL ) {
				ep_divmain = PE_ImportFnc( pefile, "W?divmain" );
			}
		}
		if( ep_divmain == NULL ) {
			PE_Free( pefile );
			dll_error = "Couldn't find DIV entrypoint";
			return NULL;
		}
		// execute entrypoint
		ep_divmain( DIV_import, DIV_export );
		// check if entrypoint was successfull
		if( dll_error != NULL ) {
			// no ? free pefile and return NULL;
			PE_Free( pefile );
			return NULL;
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
	if( ep_divlibrary != NULL ) {
		ep_divlibrary( COM_export );
		// check if entrypoint was successfull
		if( dll_error != NULL ) {
			// no ? free pefile and return NULL;
			PE_Free( pefile );
			return NULL;
		}
	}
	return pefile;
}

/*
 * Carga una DLL (int‚rprete)
 * Llama a todas las funciones (divmain, divlibrary, divend)
 */
PE* DIV_LoadDll( const char* name )
{
	return Internal_LoadDll( name, 1 );
}

/*
 * Descarga una DLL (compilador)
 */
void DIV_UnImportDll( PE* pefile )
{
	PE_Free( pefile );
}

/*
 * Carga una DLL (compilador)
 * S¢lo llama a divlibrary para saber qu‚ funciones exporta la DLL
 */
PE* DIV_ImportDll( const char* name )
{
	return Internal_LoadDll( name, 0 );
}
