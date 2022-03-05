/*
	MikDLL - Done by MikMak / HaRDCoDE '95
*/
#ifndef DIVDLL_H
#define DIVDLL_H

#include "pe_load.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void ( *COM_export_t )( const char* name, void* dir, int nparms );
typedef void ( *DIV_export_t )( const char* name, void* obj );
typedef void* ( *DIV_import_t )( const char* name );
typedef void ( *divmain_t )( DIV_import_t, DIV_export_t );
typedef void ( *divlibrary_t )( COM_export_t );
typedef void ( *divend_t )( DIV_import_t, DIV_export_t );

void DIV_RemoveExport( const char* name, void* obj );

PE*	 DIV_LoadDll( const char* name );
PE*	 DIV_ImportDll( const char* name );
void DIV_UnLoadDll( PE* pefile );
void DIV_UnImportDll( PE* pefile );

#ifndef DIVDLL_C
extern COM_export_t COM_export;
#endif

void  DIV_export( const char* name, void* obj );
void* DIV_import( const char* name );

#ifdef __cplusplus
}
#endif

#endif
