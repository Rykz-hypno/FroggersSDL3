#pragma once
#include "SDL3/SDL_render.h"
#include "gamestate.h"

bool TryMove(Entity* mover, LevelData* level, CommandBuffer* cmd_buffer, int xDir, int yDir, uint32_t timestamp);
Entity* UseTongue(Entity* player, LevelData* level, CommandBuffer* cmd_buffer, uint32_t timestamp);

void ChangeScene(GameData* data, SCENE_TYPES new_scene);

extern "C" {
  __declspec(dllexport) void Initialize(GameData* data, SDL_Window* window, SDL_Renderer* renderer);
  __declspec(dllexport) bool HandleEvents(GameData* data, SDL_Event event);
  __declspec(dllexport) void Draw(GameData* data, SDL_Renderer* renderer);
  __declspec(dllexport) void Update(GameData* data, float dt);
  __declspec(dllexport) void OnQuit(SDL_Renderer* renderer);
}
