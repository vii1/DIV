#ifndef __WLD_H__
#define __WLD_H__

#include "vpe.h"

typedef struct _wld_point {
  uint32_t active;
  int32_t x;
  int32_t y;
  uint32_t links;
} wld_point_t;

typedef struct _wld_wall {
  uint32_t active;
  int32_t type;
  int32_t p_start;
  int32_t p_end;
  int32_t r_front;
  int32_t r_back;
  int32_t t_mid;
  int32_t t_top;
  int32_t t_bot;
  uint32_t fade;
} wld_wall_t;

typedef struct _wld_region {
  uint32_t active;
  int32_t type;
  int32_t h_floor;
  int32_t h_ceil;
  int32_t t_floor;
  int32_t t_ceil;
  uint32_t fade;
} wld_region_t;

typedef struct _wld_flag {
  uint32_t active;
  int32_t x;
  int32_t y;
  int32_t number;
} wld_flag_t;

typedef struct _wld_info {
  char wld_path[256];
  char wld_name[16];
  int32_t number;
  char fpg_path[256];
  char fpg_name[16];
} wld_info_t;

typedef struct _wld_map {
  wld_info_t info;

  int32_t num_points;
  wld_point_t* points;

  int32_t num_walls;
  wld_wall_t* walls;

  int32_t num_regions;
  wld_region_t* regions;

  int32_t num_flags;
  wld_flag_t* flags;

  int32_t background;
} wld_map_t;

/**
 * Estructura principal
 */
typedef struct _wld {
  char signature[8];
  uint32_t vpe_offset;

  wld_map_t wld;
  vpe_map_t vpe;
} wld_t;

void wld_print_info( wld_info_t* info);
void wld_print(wld_t* wld);
int wld_load(wld_t* wld, const char* filename);
//void wld_save(wld_t* wld);

#endif
