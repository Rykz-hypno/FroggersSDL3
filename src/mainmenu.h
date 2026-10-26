#pragma once

struct GameData;
struct SDL_Renderer;
struct Button;
struct Sprite;
struct FontAtlas;
struct Input;
namespace Memory{
  struct Arena;
}

struct MainMenu{
  Button* buttons;
  int button_count;
  int activeButtonIndex;
  bool initialized;
};

void InitializeMenu(MainMenu* mainmenu, Sprite* spriteBuffer, FontAtlas* font, Memory::Arena* arena);
void UpdateMenu(MainMenu* mainmenu, GameData* data);
void DrawMenu(MainMenu* mainmenu, SDL_Renderer* renderer, Sprite* spriteBuffer, FontAtlas* title_font, FontAtlas* subtitle_font, Input* input);
