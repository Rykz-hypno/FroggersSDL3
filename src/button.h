#pragma once
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "rendering.h"

struct Sprite;
struct GameData;
struct FontAtlas;

enum class ButtonType{
  NONE,
  START_GAME,
  QUIT
};

struct Button{
  ButtonType type;
  SDL_FRect rect;
  Sprite* sprite;
  bool is_active;
  FontAtlas* font;
  const char* text;
};

void PressButton(Button* button, GameData* data);
bool IsHoveredOver(Button* button, float x, float y);
void SetupButton(Button* button,
                 ButtonType type,
                 Sprite* spriteBuffer,
                 SDL_FRect rect,
                 Alignment mode,
                 FontAtlas* font = nullptr,
                 const char* text = nullptr);
