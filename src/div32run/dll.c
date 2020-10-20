#include <dos.h>
#include "divdll.h"

PE*	  pe[128];
int	  nDLL;
void* ExternDirs[1024];

void LookForAutoLoadDlls()
{
	struct find_t dllfiles;
	int			  ct;
	ct = _dos_findfirst( "*.DLL", _A_NORMAL, &dllfiles );
	nDLL = 0;
	while( ct == 0 ) {
		pe[nDLL] = DIV_LoadDll( dllfiles.name );
		if( pe[nDLL] != NULL ) {
			if( DIV_import( "Autoload" ) == NULL )
				DIV_UnLoadDll( pe[nDLL] );
			else
				nDLL++;
		}
		ct = _dos_findnext( &dllfiles );
	}
}

#ifdef PEDEBUG
#include "../pedebug.c"
#endif
