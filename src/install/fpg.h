#ifndef __FPG_H__
#define __FPG_H__

#include "main.h"

typedef struct {
	dword code;
	dword length;
	char  description[32];
	char  filename[12];
	dword width;
	dword height;
	dword numCpoints;
	_Packed struct {
		short x, y;
	} cpoints[];
} FpgMapHeader;

extern FpgMapHeader** fpgIndex;
extern int			  fpgIndexSize;
extern byte*		  installFpg;

void cargar_fpg( const byte* buf, unsigned int uSize, unsigned int zSize );
void verificar_fpg();
const byte* fpg_map( int code );

#endif	 // __FPG_H__
