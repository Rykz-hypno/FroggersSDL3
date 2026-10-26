#pragma once

#include "camera.h"
#include "input.h"
#include "spriteLibrary.h"
#include "levels.h"
#include "command.h"
#include "SDL3/SDL_render.h"

struct Editor{
  ID object_to_place_id;
};

namespace EDITOR{
  void DrawObjectPanel(Editor* editor, Sprite* spriteBuffer);
  void PlaceObject(const int x, const int y, Editor* editor, LevelData* level, CommandBuffer* buffer, uint32_t* timestamp);
  void Update(Editor* editor, Input* input, LevelData* level, CommandBuffer* buffer, uint32_t* timestamp);
  void DrawPreview(Editor* editor, Input* input, SDL_Renderer* renderer, LevelData* level, Camera* camera, Sprite* spriteBuffer);
}
