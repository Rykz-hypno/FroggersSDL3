#include "spriteLibrary.h"
#include "SDL3_image/SDL_image.h"
#include <cassert>

const char* FALLBACK_PATH = "assets/sprites/fallback.png";

static const SpriteDataEntry all_sprite_data[] = {
  {SPRITE_ID::Fallback, FALLBACK_PATH               },
  {SPRITE_ID::Wall,     "assets/sprites/wall.png"   },
  {SPRITE_ID::Player,   "assets/sprites/player.png" },
  {SPRITE_ID::Box,     "assets/sprites/box.png"   },
  {SPRITE_ID::Ground,   "assets/sprites/ground.png" },
  {SPRITE_ID::Dropshadow, "assets/sprites/dropshadow.png" },
  {SPRITE_ID::Goal,     "assets/sprites/goal.png"   },
  {SPRITE_ID::Titlescreen_background, "assets/sprites/titlescreen.png", 0, 0},
  {SPRITE_ID::Black_1x1, "assets/sprites/1x1_black.png", 0, 0},
  {SPRITE_ID::Button_Basic, "assets/sprites/basic_button.png", 0, 0}
};

Sprite* GetSpriteFromID(ID id, Sprite* spriteBuffer){
  Sprite* sprite_to_return = nullptr;

  switch (id) {
    case ID::NONE:
      sprite_to_return = nullptr;
      break;
    case ID::GROUND:
      sprite_to_return = &spriteBuffer[(int)SPRITE_ID::Ground];
      break;
    case ID::WALL:
      sprite_to_return = &spriteBuffer[(int)SPRITE_ID::Wall];
      break;
    case ID::PLAYER:
      sprite_to_return = &spriteBuffer[(int)SPRITE_ID::Player];
      break;
    case ID::BOX:
      sprite_to_return = &spriteBuffer[(int)SPRITE_ID::Box];
      break;
    case ID::GOAL:
      sprite_to_return = &spriteBuffer[(int)SPRITE_ID::Goal];
      break;
  }

  if(sprite_to_return == nullptr || sprite_to_return->texture == nullptr){
    sprite_to_return = &spriteBuffer[(int)SPRITE_ID::Fallback];
  }

  return sprite_to_return;
}

Sprite* GetSprite(SPRITE_ID sprite_id, Sprite* spriteBuffer){
  return &spriteBuffer[(int)sprite_id];
}

namespace AssetManagement{
  void LoadAllSprites(Sprite* spriteBuffer, SDL_Renderer* renderer){
    for (SpriteDataEntry entry : all_sprite_data) {
      LoadSprite(spriteBuffer, entry, renderer);
    }
  }

  void LoadSprite(Sprite* spriteBuffer, SpriteDataEntry entry, SDL_Renderer* renderer){
    SDL_Surface* surface = IMG_Load(entry.path);
    if(surface == nullptr){
      surface = IMG_Load(FALLBACK_PATH);
    }
    assert(surface != nullptr);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    Sprite* sprite = &spriteBuffer[(int)entry.id];
    sprite->texture = texture;
    sprite->height = texture->h;
    sprite->width = texture->w;

    if(entry.pivot_x == NOT_SET || entry.pivot_y == NOT_SET){
      sprite->pivot_x = sprite->width / 2;
      sprite->pivot_y = sprite->height / 2;
    }
    else{
      sprite->pivot_x = entry.pivot_x;
      sprite->pivot_y = entry.pivot_y;
    }

    SDL_DestroySurface(surface);
  }
}
