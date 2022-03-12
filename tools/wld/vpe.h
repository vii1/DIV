#ifndef __VPE_H__
#define __VPE_H__

#include <stdint.h>
#include <stdlib.h>

typedef struct _vpe_point {
  uint32_t type;
  int32_t x;
  int32_t y;
  int16_t path;
  int16_t link;
} vpe_point_t;

typedef struct _vpe_wall {
  uint32_t type;
  int16_t p_start;
  int16_t p_end;
  int16_t r_front;
  int16_t r_back;
  int32_t t_top;
  int32_t t_mid;
  int32_t t_bot;
  char effect[10];
  int16_t fade;
  int16_t t_x;
  int16_t t_y;
  int16_t mass;
  int16_t tag;
} vpe_wall_t;

typedef struct _vpe_region {
  uint32_t type;
  int16_t h_floor;
  int16_t h_ceil;
  int16_t r_below;
  int16_t r_above;
  int32_t t_floor;
  int32_t t_ceil;
  char effect[10];
  int16_t fade;
  int16_t tag;
} vpe_region_t;

typedef struct _vpe_flag {
  int32_t x;
  int32_t y;
  int32_t number;
} vpe_flag_t;

typedef struct _vpe_move {
  int16_t x;
  int16_t y;
  int16_t z;
  int16_t t;
} vpe_move_t;

typedef struct _vpe_info {
  char title[24];
  char palette[12];
  int32_t screen_tex;
  int32_t back_tex;
  char back_effect[10];
  int16_t back_angle;
  int16_t act_view;

  vpe_move_t force;
} vpe_info_t;

typedef struct _vpe {
  char signature[4];
  int16_t num_points;
  int16_t num_regions;
  int16_t num_walls;
  int16_t num_flags;
  vpe_point_t* points;
  vpe_region_t* regions;
  vpe_wall_t* walls;
  vpe_flag_t* flags;
  vpe_info_t info;
} vpe_map_t;

void vpe_print_wall( vpe_wall_t* wall, uint32_t index);
void vpe_print_region( vpe_region_t* region, uint32_t index);
void vpe_print_point( vpe_point_t* point, uint32_t index);
void vpe_print_flag( vpe_flag_t* flag, uint32_t index);
void vpe_print_move( vpe_move_t* move);
void vpe_print_info( vpe_info_t* info);
void vpe_map_load(vpe_map_t* map, FILE* f);

#endif
