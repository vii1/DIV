#ifndef PE_LOADER_H
#define PE_LOADER_H

#include <stdio.h>

// Declare PE type as void *.. we don't want to bother the calling program
// with the complex PE defines..
typedef void PE;

void  PE_Free( PE* p );
PE*	  PE_ReadFP( FILE* in );
PE*	  PE_ReadFN( const char* filename );
void* PE_ImportFnc( PE* p, const char* funcname );

extern char* dll_error;

#endif	 // PE_LOADER_H
