#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "vpe.h"

void vpe_print_wall( vpe_wall_t* wall, uint32_t index) {
  printf("VPE wall %d\n", index);
  printf("  type: %d\n", wall->type);
  printf("  point start: %d\n", wall->p_start);
  printf("  point end: %d\n", wall->p_end);
  printf("  region front: %d\n", wall->r_front);
  printf("  region back: %d\n", wall->r_back);
  printf("  texture top: %d\n", wall->t_top);
  printf("  texture middle: %d\n", wall->t_mid);
  printf("  texture bottom: %d\n", wall->t_bot);
  printf("  effect: %s\n", wall->effect);
  printf("  fade: %d\n", wall->fade);
  printf("  texture x: %d\n", wall->t_x);
  printf("  texture y: %d\n", wall->t_y);
  printf("  mass: %d\n", wall->mass);
  printf("  tag: %d\n", wall->tag);
}

void vpe_print_region( vpe_region_t* region, uint32_t index) {
  printf("VPE region %d\n", index);
  printf("  type: %d\n", region->type);
  printf("  height floor: %d\n", region->h_floor);
  printf("  height ceil: %d\n", region->h_ceil);
  printf("  region below: %d\n", region->r_below);
  printf("  region above: %d\n", region->r_above);
  printf("  texture floor: %d\n", region->t_floor);
  printf("  texture ceil: %d\n", region->t_ceil);
  printf("  effect: %s\n", region->effect);
  printf("  fade: %d\n", region->fade);
  printf("  tag: %d\n", region->tag);
}

void vpe_print_point( vpe_point_t* point, uint32_t index) {
  printf("VPE point %d\n", index);
  printf("  type: %d\n", point->type);
  printf("  x: %d\n", point->x);
  printf("  y: %d\n", point->y);
  printf("  path: %d\n", point->path);
  printf("  link: %d\n", point->link);
}

void vpe_print_flag( vpe_flag_t* flag, uint32_t index) {
  printf("VPE flag %d\n", index);
  printf("  x: %d\n", flag->x);
  printf("  y: %d\n", flag->y);
  printf("  number: %d\n", flag->number);
}

void vpe_print_move( vpe_move_t* move) {
  printf("VPE move\n");
  printf("  x: %d\n", move->x);
  printf("  y: %d\n", move->y);
  printf("  z: %d\n", move->z);
  printf("  t: %d\n", move->t);
}

void vpe_print_info( vpe_info_t* info) {
  printf("VPE info\n");
  printf("  title: %s\n", info->title);
  printf("  palette: %s\n", info->palette);
  printf("  screen_tex: %d\n", info->screen_tex);
  printf("  back_tex: %d\n", info->back_tex);
  printf("  back_effect: %s\n", info->back_effect);
  printf("  back_angle: %d\n", info->back_angle);
  printf("  act_view: %d\n", info->act_view);

  vpe_print_move(&info->force);
}

void vpe_map_load(vpe_map_t* vpe, FILE* f) {
  fread(&vpe->signature, sizeof(vpe->signature), 1, f);

  fread(&vpe->num_points, sizeof(vpe->num_points), 1, f);
  fread(&vpe->num_regions, sizeof(vpe->num_regions), 1, f);
  fread(&vpe->num_walls, sizeof(vpe->num_walls), 1, f);
  fread(&vpe->num_flags, sizeof(vpe->num_flags), 1, f);

  vpe->points = calloc(vpe->num_points, sizeof(vpe_point_t));
  fread(vpe->points, sizeof(vpe_point_t), vpe->num_points, f);

  // Leemos las regions.
  vpe->regions = calloc(vpe->num_regions, sizeof(vpe_region_t));
  fread(vpe->regions, sizeof(vpe_region_t), vpe->num_regions, f);

  // Leemos las paredes.
  vpe->walls = calloc(vpe->num_walls, sizeof(vpe_wall_t));
  fread(vpe->walls, sizeof(vpe_wall_t), vpe->num_walls, f);

  // Leemos las banderas.
  vpe->flags = calloc(vpe->num_flags, sizeof(vpe_flag_t));
  fread(vpe->flags, sizeof(vpe_flag_t), vpe->num_flags, f);

  // Leemos la info extra al final del archivo.
  fread(&vpe->info, sizeof(vpe->info), 1, f);
}

