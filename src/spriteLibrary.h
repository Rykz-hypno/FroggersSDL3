#pragma once
#include "SDL3/SDL_render.h"
#include "entity.h"

struct Sprite{
  SDL_Texture* texture;
  int width;
  int height;
  int pivot_x;
  int pivot_y;
};

enum class SPRITE_ID{
  Fallback,
  Ground,
  Wall,
  Box,
  Player,
  Dropshadow,
  Goal,
  Titlescreen_background,
  Black_1x1,
  Button_Basic
};

const int NOT_SET = -1;

struct SpriteDataEntry{
  SPRITE_ID id;
  const char* path;
  int pivot_x = NOT_SET;
  int pivot_y = NOT_SET;
};

Sprite* GetSpriteFromID(ID id, Sprite* spriteBuffer);
Sprite* GetSprite(SPRITE_ID sprite_id, Sprite* spriteBuffer);

namespace AssetManagement
{
  void LoadSprite(Sprite* spriteBuffer, SpriteDataEntry entry, SDL_Renderer* renderer);
  void LoadAllSprites(Sprite* spriteBuffer, SDL_Renderer* renderer);
}
