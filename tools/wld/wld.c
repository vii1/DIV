#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wld.h"

#define ERR_OPEN 1
#define ERR_WLD_SIGNATURE 2
#define ERR_VPE_SIGNATURE 3

void wld_print_info(wld_info_t* info) {
  printf("WLD info\n");
  printf("  WLD path: %s\n", info->wld_path);
  printf("  WLD name: %s\n", info->wld_name);
  printf("  number: %d\n", info->number);
  printf("  FPG path: %s\n", info->fpg_path);
  printf("  FPG name: %s\n", info->fpg_name);
}

void wld_print(wld_t* wld) {
  printf("VPE offset: %d\n", wld->vpe_offset);
  wld_print_info(&wld->wld.info);
}

int wld_load(wld_t* wld, const char* filename) {
  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    return ERR_OPEN;
  }

  // Comprobamos la firma del archivo. 
  fread(wld->signature, sizeof(wld->signature), 1, f);
  if (strncmp(wld->signature, "wld\x1A\x0D\x0A\x01\x00", 8) != 0) {
    fclose(f);
    return ERR_WLD_SIGNATURE;
  }

  // Offset de la estructura VPE
  fread(&wld->vpe_offset, sizeof(uint32_t), 1, f);

  // Datos de WLD
  fread(&wld->wld.info, sizeof(wld_info_t), 1, f);

  // Leemos los puntos.
  fread(&wld->wld.num_points, sizeof(wld->wld.num_points), 1, f);
  wld->wld.points = calloc(wld->wld.num_points, sizeof(wld_point_t));
  fread(wld->wld.points, sizeof(wld_point_t), wld->wld.num_points, f);

  // Leemos las paredes.
  fread(&wld->wld.num_walls, sizeof(wld->wld.num_walls), 1, f);
  wld->wld.walls = calloc(wld->wld.num_walls, sizeof(wld_wall_t));
  fread(wld->wld.walls, sizeof(wld_wall_t), wld->wld.num_walls, f);

  // Leemos las regions.
  fread(&wld->wld.num_regions, sizeof(wld->wld.num_regions), 1, f);
  wld->wld.regions = calloc(wld->wld.num_regions, sizeof(wld_region_t));
  fread(wld->wld.regions, sizeof(wld_region_t), wld->wld.num_regions, f);

  // Leemos las banderas.
  fread(&wld->wld.num_flags, sizeof(wld->wld.num_flags), 1, f);
  wld->wld.flags = calloc(wld->wld.num_flags, sizeof(wld_flag_t));
  fread(wld->wld.flags, sizeof(wld_flag_t), wld->wld.num_flags, f);

  // Leemos VPE
  fseek(f, wld->vpe_offset + 12, SEEK_SET);

  // Leemos los datos del VPE.
  vpe_map_load(&wld->vpe, f);
  if (strncmp(wld->vpe.signature, "DAT\x00", 4) != 0) {
    fclose(f);
    return ERR_VPE_SIGNATURE;
  }

  fclose(f);
  return 0;
}
