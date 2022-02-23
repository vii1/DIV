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

enum _InstallFpgCode {
	FPG_MOUSE = 1,
	FPG_INTRO = 2,
	FPG_FONDO = 3,
	FPG_BOTON_INSTALAR_ON = 4,
	FPG_BOTON_INSTALAR_OFF = 5,
	FPG_BOTON_AYUDA_ON = 6,
	FPG_BOTON_AYUDA_OFF = 7,
	FPG_BOTON_SALIR_ON = 8,
	FPG_BOTON_SALIR_OFF = 9,
	FPG_UNIDAD_ON = 10,
	FPG_UNIDAD_OFF = 11,
	FPG_VENTANA_AYUDA = 12,
	FPG_VENTANA_RUTA = 13,
	FPG_VENTANA_PROGRESO = 14,
	FPG_BARRA_PROGRESO = 15,
	FPG_VENTANA_INFO = 16,
	FPG_VENTANA_ERROR = 17
};

extern FpgMapHeader** fpgIndex;
extern int			  fpgIndexSize;
extern byte*		  installFpg;

void cargar_fpg( const byte* buf, unsigned int uSize, unsigned int zSize );
void verificar_fpg();
const byte* fpg_map( int code );
void fpg_map_center( int code, int* cx, int* cy );

#endif	 // __FPG_H__
