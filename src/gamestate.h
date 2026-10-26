#pragma once
#include "spriteLibrary.h"
#include "camera.h"
#include "leveleditor.h"
#include "levels.h"
#include "command.h"
#include "input.h"
#include "fontLibrary.h"
#include "mainmenu.h"
#include "audioSystem.h"
#include "imgui/imgui.h"
#include <cstdint>

enum class SCENE_TYPES : uint8_t{
  NONE,
  TITLESCREEN,
  GAME
};

struct Transition{
  enum States{
    Inactive,
    FadeTo,
    FadeFrom
  };
  States state;
  float fade_time_elapsed;
  float fade_time_duration;
};

struct GameData {
  Sprite* spriteBuffer;
  AudioSystem audio;
  FontAtlas font;
  FontAtlas font_buttons;
  FontAtlas font_subtitle;
  MainMenu mainMenu;
  bool running;
  Camera camera;
  bool edit_level;
  Editor editorData;

  Memory::Arena* arena_levels;
  Memory::Arena* arena_entities;
  Memory::Arena* arena_images;
  Memory::Arena* arena_commands;
  Memory::Arena* arena_scratch;
  Memory::Arena* arena_audio;

  LevelData* levels;
  int levelCount;
  int currentLevelIndex;
  Input input;
  Memory::Arena* arena_input;
  CommandBuffer* commandBuffer;
  uint32_t command_timestamp;
  Position* input_buffer;
  int input_buffer_capacity;
  int input_buffer_write_count;
  int input_buffer_read_count;
  Entity* tongue_owner;
  Entity* tongue_target;
  float tongue_miss_timer;
  float tongue_cooldown;
  float tongue_miss_reach;
  const float* dt;
  float* dt_scaler;

  SCENE_TYPES scene_current;
  SCENE_TYPES scene_previous;
  Transition transition;

  bool level_complete;
  float level_complete_timer;
  bool level_advance_pending;
  int next_level;
  ImGuiContext* imGui_context;

  LevelData* GetCurrentLevel(){
    return &levels[currentLevelIndex];
  }
};
