#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

enum _ErrorNum {
	E_RUTA = 0,		   // La unidad o ruta especificada no es válida
	E_INTERR = 1,	   // La instalación ha sido interrumpida por el usuario
	E_LECTURA = 2,	   // Se produjo un error en la lectura de los archivos
	E_ESCRITURA = 3,   // Se produjo un error en la escritura de los archivos
	E_ESPACIO = 4,	   // No hay suficiente espacio en la unidad seleccionada
	E_MEMORIA = 5,	   // No hay suficiente memoria para realizar la instalación
	E_FPG = 6,		   // El FPG interno no es válido (faltan gráficos o puntos de control)
	E_VESA = 7,		   // No se encontró un driver VESA (controlador de pantalla)
	E_DISCO = 8		   // No se encontró ningún disco duro en el que instalar
};
typedef enum _ErrorNum ErrorNum;

void error( ErrorNum num );
void* div_malloc( size_t size );
void descomprimir( byte* dest, unsigned int destLen, const byte* src, unsigned int srcLen );

#endif // __MAIN_H__
