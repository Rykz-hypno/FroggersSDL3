#include "levelRenderer.h"
#include "common.h"
#include "camera.h"
#include "rendering.h"
#include <cstdint>
#include <cmath>
#include <algorithm>

static bool IsEntityBelowOtherEntity(Entity* a, Entity* b){
  if(a->y != b->y){
    return a->y < b->y;
  }
  return a->x < b->x;
}

void RenderLevel(GameData* gameData, SDL_Renderer* renderer){
  LevelData* lvl = &gameData->levels[gameData->currentLevelIndex];

  for(int x = 0; x < lvl->w; x++){
    for(int y = 0; y < lvl->h; y++){
      uint8_t cellType = lvl->GetCellID(x, y);
      Sprite* sprite = GetSpriteFromID((ID)cellType, gameData->spriteBuffer);
      RenderSprite_Grid(sprite, lvl, renderer, &gameData->camera, x, y);
    }
  }
}

void RenderEntities(GameData* data, SDL_Renderer* renderer){
  LevelData* lvl = &data->levels[data->currentLevelIndex];

  Entity** sorted = ALLOC_ARRAY(data->arena_scratch, Entity*, lvl->entityCount);
  for(int i = 0; i < lvl->entityCount; i++){
    sorted[i] = &lvl->entityBuffer[i];
  }
  std::sort(sorted, sorted + lvl->entityCount, IsEntityBelowOtherEntity);

  for(int i = 0; i < lvl->entityCount; i++){
    Entity* entity = sorted[i];
    if(entity->id == ID::NONE){
      continue;
    }
    Sprite* sprite = GetSpriteFromID(entity->id, data->spriteBuffer);

    float x_animated = std::lerp((float)entity->x_prev, (float)entity->x, entity->progress_01);
    float y_animated = std::lerp((float)entity->y_prev, (float)entity->y, entity->progress_01);

    float hop = 0;
    if(entity->HasBehaviour(JUMPS) && !entity->HasBehaviour(IS_PUSHING) && IsMoving(entity)){
      hop = std::sin(entity->progress_01 * 3.14159265f) * HOP_HEIGHT_PX;
    }

    Sprite* shadow = &data->spriteBuffer[(int)SPRITE_ID::Dropshadow];
    RenderSprite_Grid(shadow, lvl, renderer, &data->camera, x_animated, y_animated, 1, 0.6);

    Camera lifted = data->camera;
    lifted.camera_y += hop;
    RenderSprite_Grid(sprite, lvl, renderer, &lifted, x_animated, y_animated);
  }
}

static void EntityCenter(GameData* data, Entity* e, float* cx, float* cy){
  LevelData* lvl = data->GetCurrentLevel();
  float x_animated = std::lerp((float)e->x_prev, (float)e->x, e->progress_01);
  float y_animated = std::lerp((float)e->y_prev, (float)e->y, e->progress_01);
  float x = x_animated;
  float y = y_animated;
  camera::GridToWorld(&x, &y, lvl);
  *cx = x - data->camera.camera_x + CELL_SIZE_PX / 2.0f;
  *cy = y - data->camera.camera_y + CELL_SIZE_PX / 2.0f;
}

void RenderTongue(GameData* data, SDL_Renderer* renderer){
  Entity* owner = data->tongue_owner;
  Entity* target = data->tongue_target;
  if(owner != nullptr && target == nullptr){
    if(data->tongue_miss_timer <= 0){
      return;
    }
    float cx, cy;
    EntityCenter(data, owner, &cx, &cy);
    float length = data->tongue_miss_reach * CELL_SIZE_PX;
    const float miss_thickness = 8.0f;
    SDL_FRect miss_rect;
    miss_rect.x = owner->dirX > 0 ? cx : cx - length;
    miss_rect.w = length;
    miss_rect.y = cy - miss_thickness / 2;
    miss_rect.h = miss_thickness;
    SDL_SetRenderDrawColor(renderer, 230, 90, 140, 255);
    SDL_RenderFillRect(renderer, &miss_rect);
    return;
  }
  if(owner == nullptr || target == nullptr){
    return;
  }
  if(!IsMoving(target)){
    return;
  }

  float ox, oy, tx, ty;
  EntityCenter(data, owner, &ox, &oy);
  EntityCenter(data, target, &tx, &ty);

  const float thickness = 8.0f;
  SDL_FRect rect;
  rect.x = std::min(ox, tx);
  rect.w = std::fabs(tx - ox);
  rect.y = oy - thickness / 2;
  rect.h = thickness;

  SDL_SetRenderDrawColor(renderer, 230, 90, 140, 255);
  SDL_RenderFillRect(renderer, &rect);
}
