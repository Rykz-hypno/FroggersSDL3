#pragma once
#include "SDL3/SDL_render.h"
#include "camera.h"
#include "spriteLibrary.h"
#include "levels.h"

struct FontAtlas;

enum class Alignment{
  Left,
  Centered
};

void RenderText(FontAtlas* atlas, const char* text, SDL_Renderer* renderer, const Camera* camera, float x, float y, Alignment alignment = Alignment::Left);

void RenderSprite_World(Sprite* sprite, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale = 1, float alpha = 1);
void RenderSprite_Grid(Sprite* sprite, LevelData* lvl, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale = 1, float alpha = 1);

struct Button;

void RenderButton_Dynamic(Button* button, bool is_selected, SDL_Renderer* renderer);
