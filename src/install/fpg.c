#include <string.h>
#include "video.h"
#include "fpg.h"

FpgMapHeader** fpgIndex;
int			   fpgIndexSize;
byte*		   installFpg;

void cargar_fpg( const byte* buf, unsigned int uSize, unsigned int zSize )
{
	byte* ptr;
	int	  i = 0;
	fpgIndexSize = 0;
	installFpg = div_malloc( uSize );
	descomprimir( installFpg, uSize, buf, zSize );
	if( memcmp( installFpg, "fpg\x1a\x0d\x0a", 7 ) ) {
		error( E_FPG );
	}
	paleta = installFpg + 8;
	ptr = installFpg + 0x548;
	while( ptr < installFpg + uSize && i < 1000 ) {
		FpgMapHeader* map = (FpgMapHeader*)ptr;
		if( map->code > fpgIndexSize ) fpgIndexSize = map->code;
		++i;
		ptr += map->length;
	}
	fpgIndexSize++;
	fpgIndex = div_malloc( fpgIndexSize * sizeof( FpgMapHeader* ) );
	memset( fpgIndex, 0, fpgIndexSize * sizeof( FpgMapHeader* ) );
	ptr = installFpg + 0x548;
	i = 0;
	while( ptr < installFpg + uSize && i < 1000 ) {
		FpgMapHeader* map = (FpgMapHeader*)ptr;
		fpgIndex[map->code] = map;
		++i;
		ptr += map->length;
	}
}

void verificar_fpg()
{
	int i;
	if( fpgIndexSize < 18 ) error( E_FPG );
	for( i = 1; i <= 17; ++i ) {
		if( !fpgIndex[i] ) error( E_FPG );
		if( i == 2 || i == 3 ) {
			if( fpgIndex[i]->width != 640 || fpgIndex[i]->height != 480 ) {
				error( E_FPG );
			}
		}
	}
}

const byte* fpg_map( int code )
{
	const FpgMapHeader* map = fpgIndex[code];
	return (const byte*)( &map->cpoints[map->numCpoints] );
}

void fpg_map_center( int code, int* cx, int* cy )
{
	FpgMapHeader* map = fpgIndex[code];
	if( map->numCpoints == 0 ) {
		if( cx ) *cx = map->width / 2;
		if( cy ) *cy = map->height / 2;
	} else {
		if( cx ) *cx = map->cpoints[0].x;
		if( cy ) *cy = map->cpoints[0].y;
	}
}