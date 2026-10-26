#include "camera.h"
#include "common.h"
#include <cmath>

bool camera::GetIsPointInsideGrid(float x, float y, const LevelData* lvl){
  int x_grid;
  int y_grid;
  WorldToGrid(x, y, &x_grid, &y_grid, lvl);
  return x_grid >= 0 && y_grid >= 0 && x_grid < lvl->w && y_grid < lvl->h;
}

void camera::GridToWorld(float* x, float* y, const LevelData* lvl){
  *x *= CELL_SIZE_PX;
  *x += SCREEN_WIDTH / 2.0;
  *x -= lvl->w * CELL_SIZE_PX / 2.0;
  *y *= CELL_SIZE_PX;
  *y += SCREEN_HEIGHT / 2.0;
  *y -= lvl->h * CELL_SIZE_PX / 2.0;
}

void camera::WorldToGrid(float x_world, float y_world, int* x, int* y, const LevelData* lvl){
  float fx = x_world;
  float fy = y_world;
  fx += lvl->w * CELL_SIZE_PX / 2.0;
  fx -= SCREEN_WIDTH / 2.0;
  fx /= CELL_SIZE_PX;
  fy += lvl->h * CELL_SIZE_PX / 2.0;
  fy -= SCREEN_HEIGHT / 2.0;
  fy /= CELL_SIZE_PX;

  *x = (int)std::floor(fx);
  *y = (int)std::floor(fy);
}
