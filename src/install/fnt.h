#ifndef __FNT_H__
#define __FNT_H__

#include "main.h"

typedef struct {
	dword width;
	dword height;
	int	  inc_y;
	dword offset;
} FntCharHeader;

typedef struct {
	byte*		   fnt;
	FntCharHeader* table;
	dword		   averageWidth;
	dword		   maxHeight;
	dword		   averageCenter;
} Fnt;

extern Fnt smallFnt;
extern Fnt bigFnt;

void cargar_fnt( Fnt* fnt, const byte* buf, unsigned int uSize, unsigned int zSize );

#endif	 // __FNT_H__
