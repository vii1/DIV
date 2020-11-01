#include <string.h>
#include "fnt.h"

Fnt smallFnt;
Fnt bigFnt;

void cargar_fnt( Fnt* fnt, const byte* buf, unsigned int uSize, unsigned int zSize )
{
	FntCharHeader* i;
	int			   numChars = 0;
	int			   totalWidth = 0;

	fnt->fnt = div_malloc( uSize );
	descomprimir( fnt->fnt, uSize, buf, zSize );
	if( memcmp( fnt->fnt, "fnt\x1a\x0d\x0a", 7 ) ) {
		error( E_FPG );
	}
	fnt->table = (FntCharHeader*)( fnt->fnt + 0x54c );
	fnt->maxHeight = 0;
	for( i = fnt->table; i < fnt->table + 256; ++i ) {
		if( i->width ) {
			++numChars;
			totalWidth += i->width;
			if( i->inc_y + i->height > fnt->maxHeight ) {
				fnt->maxHeight = i->inc_y + i->height;
			}
		}
	}
	fnt->averageWidth = totalWidth / numChars;
	fnt->averageCenter = fnt->averageWidth / 2;
}
