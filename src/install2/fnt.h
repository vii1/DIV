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

typedef enum {
	ANCHOR_NW = 0,
	ANCHOR_N = 1,
	ANCHOR_NE = 2,
	ANCHOR_W = 3,
	ANCHOR_CENTER = 4,
	ANCHOR_E = 5,
	ANCHOR_SW = 6,
	ANCHOR_S = 7,
	ANCHOR_SE = 8
} Anchor;

extern Fnt smallFnt;
extern Fnt bigFnt;

void cargar_fnt( Fnt* fnt, const byte* buf, unsigned int uSize, unsigned int zSize );

#endif	 // __FNT_H__
