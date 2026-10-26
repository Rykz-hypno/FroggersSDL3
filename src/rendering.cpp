#include "rendering.h"
#include "SDL3/SDL_render.h"
#include "common.h"
#include "fontLibrary.h"
#include "button.h"
#include <cassert>

void RenderSprite_World(Sprite* sprite, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale, float alpha, bool flip_horizontal){
  SDL_FRect rect;
  rect.x = x - sprite->pivot_x * UPSCALE_FACTOR * scale;
  rect.y = y - sprite->pivot_y * UPSCALE_FACTOR * scale;
  rect.h = sprite->height * UPSCALE_FACTOR * scale;
  rect.w = sprite->width * UPSCALE_FACTOR * scale;
  if(camera != nullptr){
    rect.x -= camera->camera_x;
    rect.y -= camera->camera_y;
  }

  SDL_SetTextureScaleMode(sprite->texture, SDL_SCALEMODE_NEAREST);
  SDL_SetTextureAlphaModFloat(sprite->texture, alpha);
  SDL_RenderTextureRotated(renderer, sprite->texture, NULL, &rect, 0.0, NULL, flip_horizontal ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void RenderSprite_Grid(Sprite* sprite, LevelData* lvl, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale, float alpha, bool flip_horizontal){
  camera::GridToWorld(&x, &y, lvl);

  x += CELL_SIZE_PX / 2.0;
  y += CELL_SIZE_PX / 2.0;
  RenderSprite_World(sprite, renderer, camera, x, y, scale, alpha, flip_horizontal);
}

void RenderText(FontAtlas* atlas, const char* text, SDL_Renderer* renderer, const Camera* camera, float x, float y, Alignment mode){
  assert(atlas->atlasTexture != nullptr);
  float draw_position_x = x;
  float draw_position_y = y;
  if(camera != nullptr){
    draw_position_x -= camera->camera_x;
    draw_position_y -= camera->camera_y;
  }

  if(mode == Alignment::Centered){
    float totalWidth = 0;
    for(int i = 0; text[i] != STOP_CHAR; i++){
      unsigned char c = (unsigned char)text[i];
      if(c >= FontAtlas::GLYPH_COUNT) continue;
      totalWidth += atlas->glyphs[c].atlasPosition.w;
    }
    draw_position_x -= totalWidth / 2.0f;
  }

  for(int i = 0; text[i] != STOP_CHAR; i++){
    unsigned char c = (unsigned char)text[i];
    if(c >= FontAtlas::GLYPH_COUNT) continue;
    Glyph glyph = atlas->glyphs[c];
    SDL_FRect renderRectangle = {draw_position_x, draw_position_y, glyph.atlasPosition.w, glyph.atlasPosition.h};
    SDL_RenderTexture(renderer, atlas->atlasTexture, &glyph.atlasPosition, &renderRectangle);
    draw_position_x += glyph.atlasPosition.w;
  }
}

void RenderButton_Dynamic(Button* button, bool is_selected, SDL_Renderer* renderer){
  uint8_t colorOverlay = is_selected ? 255 : 200;
  SDL_Texture* texture = button->sprite->texture;
  SDL_FRect rect = button->rect;

  float part_w = button->sprite->width / 3.0f;
  float part_h = button->sprite->height / 3.0f;

  float corner_w = part_w * UPSCALE_FACTOR;
  float corner_h = part_h * UPSCALE_FACTOR;

  float center_w = rect.w - corner_w * 2;
  float center_h = rect.h - corner_h * 2;
  float right_x = rect.x + rect.w - corner_w;
  float bottom_y = rect.y + rect.h - corner_h;
  float center_x = rect.x + corner_w;
  float center_y = rect.y + corner_h;

  SDL_FRect dst[9] = {
    {rect.x,   rect.y,   corner_w, corner_h},
    {center_x, rect.y,   center_w, corner_h},
    {right_x,  rect.y,   corner_w, corner_h},
    {rect.x,   center_y, corner_w, center_h},
    {center_x, center_y, center_w, center_h},
    {right_x,  center_y, corner_w, center_h},
    {rect.x,   bottom_y, corner_w, corner_h},
    {center_x, bottom_y, center_w, corner_h},
    {right_x,  bottom_y, corner_w, corner_h},
  };
  SDL_FRect src[9];
  for(int row = 0; row < 3; row++){
    for(int col = 0; col < 3; col++){
      src[row * 3 + col] = {part_w * col, part_h * row, part_w, part_h};
    }
  }

  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureColorMod(texture, colorOverlay, colorOverlay, colorOverlay);
  for(int i = 0; i < 9; i++){
    SDL_RenderTexture(renderer, texture, &src[i], &dst[i]);
  }

  if(!IsStringEmpty(button->text)){
    float glyph_height = button->font->glyphs['H'].atlasPosition.h / 2.0f;
    RenderText(button->font, button->text, renderer, nullptr, rect.x + rect.w / 2.0f,
               rect.y + rect.h / 2.0f - glyph_height, Alignment::Centered);
  }
}
