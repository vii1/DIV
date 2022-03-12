#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vpe.h"

int main(int argc, char** argv) {
  uint32_t vpe_offset, vpe_size;
  char* vpe_buffer;
  vpe_map_t* vpe_map;

  //
  if (argc < 3) {
    printf("Usage: %s <input> <output>\n", argv[0]);
    return 1;
  }

  // Abrimos el archivo de entrada.
  FILE* ifile = fopen(argv[1], "rb");
  if (ifile == NULL) {
    fprintf(stderr, "Cannot read %s\n", argv[1]);
    return 1;
  }
 
  // Abrimos el archivo de salida.
  FILE* ofile = fopen(argv[2], "wb");
  if (ofile == NULL) {
    fclose(ifile);
    fprintf(stderr, "Cannot write %s\n", argv[2]);
    return 1;
  }

  // Calculamos el tamaño del archivo.
  fseek(ifile, 0, SEEK_END);
  int isize = ftell(ifile);

  // Vamos al offset de VPE.
  fseek(ifile, 8, SEEK_SET);

  // Leemos el offset de VPE.
  fread(&vpe_offset, sizeof(vpe_offset), 1, ifile);

  // le sumamos 12.
  vpe_offset += 12;
  vpe_size = isize - vpe_offset;
  vpe_buffer = malloc(vpe_size);

  // Vamos a esta parte.
  fseek(ifile, vpe_offset, SEEK_SET);
  fread(vpe_buffer, vpe_size, 1, ifile);

  // Corregimos la cabecera del archivo.
  vpe_buffer[0] = 'Z';
  vpe_buffer[1] = 'F';
  vpe_buffer[2] = '6';

  // Volcamos todo el código que hemos obtenido de VPE.
  fwrite(vpe_buffer, vpe_size, 1, ofile);

  // Asignamos la dirección de memoria del vpe_map a la dirección de memoria del vpe_buffer;
  fclose(ifile);
  return 0;
}
