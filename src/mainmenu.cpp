#include "mainmenu.h"
#include "SDL3/SDL_scancode.h"
#include "arena.h"
#include "button.h"
#include "common.h"
#include "fontLibrary.h"
#include "gamestate.h"
#include "input.h"
#include "rendering.h"
#include "spriteLibrary.h"
#include <cassert>

void InitializeMenu(MainMenu* mainmenu, Sprite* spriteBuffer, FontAtlas* font, Memory::Arena* arena){
  if(mainmenu->initialized) return;

  mainmenu->button_count = 2;
  mainmenu->buttons = ALLOC_ARRAY(arena, Button, mainmenu->button_count);

  SetupButton(&mainmenu->buttons[0], ButtonType::START_GAME, spriteBuffer,
              {SCREEN_WIDTH / 2.0f, 215, 150, 46}, Alignment::Centered, font, "Start");
  SetupButton(&mainmenu->buttons[1], ButtonType::QUIT, spriteBuffer,
              {SCREEN_WIDTH / 2.0f, 275, 150, 46}, Alignment::Centered, font, "Quit");

  mainmenu->activeButtonIndex = 0;
  mainmenu->initialized = true;
}

void UpdateMenu(MainMenu* mainmenu, GameData* data){
  Input* input = &data->input;
  int count = mainmenu->button_count;

  if(data->transition.state != Transition::Inactive) return;

  if(KeyPressed(input, SDL_SCANCODE_UP) || KeyPressed(input, SDL_SCANCODE_W)){
    mainmenu->activeButtonIndex = (mainmenu->activeButtonIndex + count - 1) % count;
  }
  if(KeyPressed(input, SDL_SCANCODE_DOWN) || KeyPressed(input, SDL_SCANCODE_S)){
    mainmenu->activeButtonIndex = (mainmenu->activeButtonIndex + 1) % count;
  }

  bool mouse_pressed_on_button = false;
  for(int i = 0; i < count; i++){
    if(IsHoveredOver(&mainmenu->buttons[i], input->mouse_x, input->mouse_y)){
      mainmenu->activeButtonIndex = i;
      if(MousePressed(input, MouseButtons::LEFT)){
        mouse_pressed_on_button = true;
      }
    }
  }

  if(mouse_pressed_on_button ||
     KeyPressed(input, SDL_SCANCODE_RETURN) || KeyPressed(input, SDL_SCANCODE_SPACE)){
    PressButton(&mainmenu->buttons[mainmenu->activeButtonIndex], data);
  }
}

void DrawMenu(MainMenu* mainmenu, SDL_Renderer* renderer, Sprite* spriteBuffer, FontAtlas* title_font, FontAtlas* subtitle_font, Input* input){
  RenderSprite_World(GetSprite(SPRITE_ID::Titlescreen_background, spriteBuffer), renderer, nullptr, 0, 0);
  RenderText(title_font, "FROGGERS", renderer, nullptr, SCREEN_WIDTH / 2.0f, 60, Alignment::Centered);
  RenderText(subtitle_font, "What's Sokoban without boxes?", renderer, nullptr, SCREEN_WIDTH / 2.0f, 90, Alignment::Centered);

  for(int i = 0; i < mainmenu->button_count; i++){
    RenderButton_Dynamic(&mainmenu->buttons[i], i == mainmenu->activeButtonIndex, renderer);
  }
}
