#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "vpe.h"
#include "wld.h"

/**
 * Principal
 */
int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s <file>\n", argv[0]);
    return 1;
  }

  // Instancia de WLD.
  wld_t wld;

  // Cargamos el archivo WLD.
  wld_load(&wld, argv[1]);
  wld_print(&wld);

  // Operamos sobre el archivo.  
  int i, j, k = 0;
  uint8_t type;
  uint32_t index;
  for (i = 2; i < argc; i++) {
    if (i % 2 == 0) {
      if (strncmp(argv[i], "region", 6) == 0) {
        type = 0;
      } else if (strncmp(argv[i], "wall", 4) == 0) {
        type = 1;
      } else if (strncmp(argv[i], "point", 5) == 0) {
        type = 2;
      } else if (strncmp(argv[i], "flag", 4) == 0) {
        type = 3;
      } else if (strncmp(argv[i], "fullregion", 10) == 0) {
        type = 4;
      } else {
        fprintf(stderr, "Unknown type %s\n", argv[i]);
        return 1;
      }
    } else {
      index = atoi(argv[i]);
      switch (type) {
        case 0: 
          vpe_print_region(&wld.vpe.regions[index], index);
          printf("VPE region walls: ");
          k = 0;
          for (j = 0; j < wld.vpe.num_walls; j++) {
            if (wld.vpe.walls[j].r_front == index) {
              if (k > 0) {
                printf(", ");
              }
              k++;
              printf("%d", j);
            }
          }
          printf("\n");
          break;
        case 1: 
          vpe_print_wall(&wld.vpe.walls[index], index);
          break;
        case 2: 
          vpe_print_point(&wld.vpe.points[index], index);
          break;
        case 3: 
          vpe_print_flag(&wld.vpe.flags[index], index);
          break;
        case 4:
          vpe_print_region(&wld.vpe.regions[index], index);
          printf("VPE region walls:\n");
          for (j = 0; j < wld.vpe.num_walls; j++) {
            if (wld.vpe.walls[j].r_front == index) {
              vpe_print_wall(&wld.vpe.walls[j], j);
              vpe_print_point(&wld.vpe.points[wld.vpe.walls[index].p_start], wld.vpe.walls[index].p_start);
              vpe_print_point(&wld.vpe.points[wld.vpe.walls[index].p_end], wld.vpe.walls[index].p_end);
            }
          }
          break;
      }
    }
  }
  return 0;
}
