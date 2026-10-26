#include "game.h"
#include "levelRenderer.h"
#include "rendering.h"
#include "command.h"
#include "input.h"
#include "dev_gui.h"
#include "common.h"
#include "mainmenu.h"
#include "button.h"
#include "audioSystem.h"
#include <cstring>
#include <cassert>

bool TryMove(Entity* mover, LevelData* level, CommandBuffer* cmd_buffer, int xDir, int yDir, uint32_t timestamp){
  if(mover->HasBehaviour(CAN_MOVE) == false){
    return false;
  }

  int test_x = mover->x + xDir;
  int test_y = mover->y + yDir;
  if(test_x < 0 || test_x >= level->w || test_y < 0 || test_y >= level->h){
    return false;
  }

  Entity* stepInto_entity = level->GetEntity(test_x, test_y);
  ID stepInto_tile_id = (ID)level->GetCellID(test_x, test_y);

  if(stepInto_entity == nullptr){
    if(IsWalkable((uint8_t)stepInto_tile_id)){
      MoveCommand mv;
      mv.type = CMD_TYPE::MOVE;
      mv.entity = mover;
      mv.xDir = xDir;
      mv.yDir = yDir;
      Push(cmd_buffer, mv, level, timestamp);
      return true;
    }
    return false;
  }

  if(stepInto_entity->HasBehaviour(CAN_MOVE)){
    if(TryMove(stepInto_entity, level, cmd_buffer, xDir, yDir, timestamp)){
      mover->AddBehaviour(IS_PUSHING);
      MoveCommand mv;
      mv.type = CMD_TYPE::MOVE;
      mv.entity = mover;
      mv.xDir = xDir;
      mv.yDir = yDir;
      Push(cmd_buffer, mv, level, timestamp);
      return true;
    }
  }
  return false;
}

Entity* UseTongue(Entity* player, LevelData* level, CommandBuffer* cmd_buffer, uint32_t timestamp){
  int dirX = player->dirX;
  if(dirX == 0){
    return nullptr;
  }
  int x = player->x + dirX;
  int y = player->y;

  while(x >= 0 && x < level->w && y >= 0 && y < level->h && IsWalkable(level->GetCellID(x, y))){
    Entity* hit = level->GetEntity(x, y);
    if(hit != nullptr){
      if(hit->HasBehaviour(CAN_MOVE) == false || hit->HasBehaviour(IS_PLAYER)){
        return nullptr;
      }
      int new_x = x - dirX;
      int new_y = y;
      if(level->GetEntity(new_x, new_y) != nullptr){
        return nullptr;
      }
      MoveCommand mv;
      mv.type = CMD_TYPE::MOVE;
      mv.entity = hit;
      mv.xDir = -dirX;
      mv.yDir = 0;
      Push(cmd_buffer, mv, level, timestamp);
      return hit;
    }
    x += dirX;
  }
  return nullptr;
}

static float TongueReach(Entity* player, LevelData* level){
  int cells = 0;
  int x = player->x + player->dirX;
  while(cells < TONGUE_RANGE && x >= 0 && x < level->w && IsWalkable(level->GetCellID(x, player->y)) && level->GetEntity(x, player->y) == nullptr){
    cells++;
    x += player->dirX;
  }
  bool blocked = cells < TONGUE_RANGE;
  return cells + (blocked ? 0.5f : 0.0f);
}

static bool LevelIsComplete(LevelData* level){
  int goals = 0;
  for(int y = 0; y < level->h; y++){
    for(int x = 0; x < level->w; x++){
      if(level->GetCellID(x, y) == (uint8_t)ID::GOAL){
        goals++;
        Entity* e = level->GetEntity(x, y);
        if(e == nullptr || e->id != ID::BOX || IsMoving(e)){
          return false;
        }
      }
    }
  }
  return goals > 0;
}

static void StartLevel(GameData* data){
  data->commandBuffer->index = 0;
  data->commandBuffer->head = 0;
  data->command_timestamp = 0;
  data->input_buffer_read_count = data->input_buffer_write_count;
  data->tongue_owner = nullptr;
  data->tongue_target = nullptr;
  data->tongue_miss_timer = 0;
  data->tongue_cooldown = 0;
  data->level_complete = false;
  data->level_complete_timer = 0;
  CreateEntities(data->GetCurrentLevel(), data->arena_entities);
}

void ChangeScene(GameData* data, SCENE_TYPES new_scene){
  assert(new_scene != data->scene_current);
  data->scene_previous = data->scene_current;
  data->scene_current = new_scene;
  data->transition.state = data->scene_previous == SCENE_TYPES::NONE ? Transition::FadeFrom : Transition::FadeTo;
  data->transition.fade_time_elapsed = 0;
  switch(data->scene_current){
    case SCENE_TYPES::TITLESCREEN:
      data->transition.fade_time_duration = 1;
      break;
    case SCENE_TYPES::GAME:
      data->transition.fade_time_duration = 0.5f;
      data->currentLevelIndex = 0;
      StartLevel(data);
      break;
    case SCENE_TYPES::NONE:
      assert(false);
      break;
  }
}

static void NextLevel(GameData* data){
  if(data->currentLevelIndex + 1 < data->levelCount){
    data->next_level = data->currentLevelIndex + 1;
    data->level_advance_pending = true;
    data->transition.state = Transition::FadeTo;
    data->transition.fade_time_elapsed = 0;
    data->transition.fade_time_duration = 0.5f;
  }
  else{
    ChangeScene(data, SCENE_TYPES::TITLESCREEN);
  }
}

static void DrawScene(GameData* data, SCENE_TYPES scene, SDL_Renderer* renderer){
  switch(scene){
    case SCENE_TYPES::TITLESCREEN:
      SDL_SetRenderViewport(renderer, nullptr);
      SDL_SetRenderScale(renderer, WINDOW_SCALE, WINDOW_SCALE);
      DrawMenu(&data->mainMenu, renderer, data->spriteBuffer, &data->font, &data->font_subtitle, &data->input);
      break;
    case SCENE_TYPES::GAME:{
      SDL_Rect content_area = {(int)(CONTENT_OFFSET_X / CONTENT_SCALE + 0.5f), (int)(CONTENT_OFFSET_Y / CONTENT_SCALE + 0.5f),
                               SCREEN_WIDTH, SCREEN_HEIGHT};
      SDL_SetRenderScale(renderer, CONTENT_SCALE, CONTENT_SCALE);
      SDL_SetRenderViewport(renderer, &content_area);
      RenderLevel(data, renderer);
      RenderEntities(data, renderer);
      RenderTongue(data, renderer);
      break;
    }
    case SCENE_TYPES::NONE:
      break;
  }
}

static void UpdateGame(GameData* data, float dt){
  if(data->tongue_miss_timer > 0){
    data->tongue_miss_timer -= dt;
  }
  if(data->tongue_cooldown > 0){
    data->tongue_cooldown -= dt;
  }
  if(KeyPressed(&data->input, SDL_SCANCODE_F2)){
    data->edit_level = !data->edit_level;
  }
  if(data->edit_level){
    EDITOR::Update(&data->editorData, &data->input, data->GetCurrentLevel(), data->commandBuffer, &data->command_timestamp);
  }

  if(KeyPressed(&data->input, SDL_SCANCODE_R) && data->level_advance_pending == false){
    StartLevel(data);
    return;
  }

  bool locked = data->level_complete;

  if(!locked && (KeyPressed(&data->input, SDL_SCANCODE_Z) || KeyHeld_ForTime(&data->input, SDL_SCANCODE_Z, UNDO_REPEAT_TIME))){
    ResetKeyHeldTime(&data->input, SDL_SCANCODE_Z);
    data->tongue_target = nullptr;
    data->tongue_miss_timer = 0;
    if(KeyHeld(&data->input, SDL_SCANCODE_LSHIFT)){
      Redo(data->commandBuffer, data->GetCurrentLevel());
    }
    else{
      Undo(data->commandBuffer, data->GetCurrentLevel());
    }
  }

  Position* buffer = data->input_buffer;
  int capacity = data->input_buffer_capacity;
  float repeat_time = (1 / MOVE_SPEED) * 1.5;
  if(locked){

  }
  else if(KeyPressed(&data->input, SDL_SCANCODE_RIGHT) || KeyHeld_ForTime(&data->input, SDL_SCANCODE_RIGHT, repeat_time)){
    ResetKeyHeldTime(&data->input, SDL_SCANCODE_RIGHT);
    buffer[data->input_buffer_write_count++ % capacity] = {1, 0};
  }
  else if(KeyPressed(&data->input, SDL_SCANCODE_LEFT) || KeyHeld_ForTime(&data->input, SDL_SCANCODE_LEFT, repeat_time)){
    ResetKeyHeldTime(&data->input, SDL_SCANCODE_LEFT);
    buffer[data->input_buffer_write_count++ % capacity] = {-1, 0};
  }
  else if(KeyPressed(&data->input, SDL_SCANCODE_UP) || KeyHeld_ForTime(&data->input, SDL_SCANCODE_UP, repeat_time)){
    ResetKeyHeldTime(&data->input, SDL_SCANCODE_UP);
    buffer[data->input_buffer_write_count++ % capacity] = {0, -1};
  }
  else if(KeyPressed(&data->input, SDL_SCANCODE_DOWN) || KeyHeld_ForTime(&data->input, SDL_SCANCODE_DOWN, repeat_time)){
    ResetKeyHeldTime(&data->input, SDL_SCANCODE_DOWN);
    buffer[data->input_buffer_write_count++ % capacity] = {0, 1};
  }
  else if(KeyPressed(&data->input, SDL_SCANCODE_SPACE)){
    buffer[data->input_buffer_write_count++ % capacity] = {0, 0};
  }

  bool are_entities_moving = false;
  for (int i = 0; i < data->GetCurrentLevel()->entityCount; i++) {
    Entity* entity = &data->GetCurrentLevel()->entityBuffer[i];
    if(entity->HasBehaviour(CAN_MOVE) && IsMoving(entity)){
      entity->progress_01 += MOVE_SPEED * dt;
      if(entity->progress_01 >= 1){
        entity->progress_01 = 0;
        entity->x_prev = entity->x;
        entity->y_prev = entity->y;
        entity->RemoveBehaviour(IS_PUSHING);
      }
      if(IsMoving(entity)){
        are_entities_moving = true;
      }
    }
  }

  if(are_entities_moving == false){

    if(data->level_complete == false && LevelIsComplete(data->GetCurrentLevel())){
      data->level_complete = true;
      data->level_complete_timer = 0;
      PlaySFX(SFX_ID::WIN);
    }
    if(data->level_complete){
      data->input_buffer_read_count = data->input_buffer_write_count;
      if(data->level_advance_pending == false){
        data->level_complete_timer += dt;
        if(data->level_complete_timer >= 0.8f){
          NextLevel(data);
        }
      }
      return;
    }

    if(data->input_buffer_read_count == data->input_buffer_write_count){
      return;
    }

    Position input = buffer[data->input_buffer_read_count % capacity];
    if(input.x == 0 && input.y == 0 && data->tongue_cooldown > 0){
      data->input_buffer_read_count++;
      return;
    }
    data->command_timestamp += 1;

    for (int i = 0; i < data->GetCurrentLevel()->entityCount; i++) {
      Entity* entity = &data->GetCurrentLevel()->entityBuffer[i];
      if(entity->HasBehaviour((Behaviour)(RESPOND_TO_INPUT | CAN_MOVE))){
        data->tongue_owner = nullptr;
        data->tongue_target = nullptr;

        if(input.x == 0 && input.y == 0){
          data->tongue_cooldown = TONGUE_COOLDOWN;
          PlaySFX(SFX_ID::TONGUE);
          Entity* pulled = UseTongue(entity, data->GetCurrentLevel(), data->commandBuffer, data->command_timestamp);
          if(pulled != nullptr){
            data->tongue_owner = entity;
            data->tongue_target = pulled;
          }
          else{
            data->tongue_owner = entity;
            data->tongue_miss_reach = TongueReach(entity, data->GetCurrentLevel());
            data->tongue_miss_timer = TONGUE_MISS_TIME;
          }
        }
        else{

          if(input.x != 0){
            entity->dirX = input.x;
          }
          entity->dirY = input.y;
          bool moved = TryMove(entity, data->GetCurrentLevel(), data->commandBuffer, input.x, input.y, data->command_timestamp);
          if(moved){
            PlaySFX(SFX_ID::STEP);
          }
        }
      }
    }
    data->input_buffer_read_count++;
  }
}

extern "C" {

void Initialize(GameData* data, SDL_Window* window, SDL_Renderer* renderer){
  DEV::Initialize(window, renderer);
  InitializeAudioSystem(&data->audio, data->arena_audio);
  AssetManagement::LoadAllSFX(&data->audio);
  data->imGui_context = ImGui::GetCurrentContext();

  AssetManagement::LoadAllSprites(data->spriteBuffer, renderer);
  AssetManagement::LoadFont(renderer, "assets/fonts/ByteBounce.ttf", &data->font, 48);
  AssetManagement::LoadFont(renderer, "assets/fonts/ByteBounce.ttf", &data->font_buttons, 36);
  AssetManagement::LoadFont(renderer, "assets/fonts/ByteBounce.ttf", &data->font_subtitle, 28);

  data->editorData.object_to_place_id = ID::WALL;
  data->currentLevelIndex = 0;
  CreateLevel(data->arena_levels, &data->levels[0], "assets/levels/firstlvl.tmj");
  CreateLevel(data->arena_levels, &data->levels[1], "assets/levels/level2.tmj");
  CreateLevel(data->arena_levels, &data->levels[2], "assets/levels/level3.tmj");
  CreateLevel(data->arena_levels, &data->levels[3], "assets/levels/level4.tmj");
  CreateLevel(data->arena_levels, &data->levels[4], "assets/levels/level5.tmj");
  data->levelCount = 5;

  SDL_SetTextureBlendMode(GetSprite(SPRITE_ID::Black_1x1, data->spriteBuffer)->texture, SDL_BLENDMODE_BLEND);

  InitializeMenu(&data->mainMenu, data->spriteBuffer, &data->font_buttons, data->arena_levels);
  ChangeScene(data, SCENE_TYPES::TITLESCREEN);
}

bool HandleEvents(GameData *data, SDL_Event event){
  DEV::ProcessEvents(&event);

  if(event.type != SDL_EVENT_KEY_DOWN){
    return true;
  }

  if(event.key.key == SDLK_ESCAPE){
    data->running = false;
    return false;
  }
  return true;
}

void Update(GameData* data, float dt){
  UpdateAudio(&data->audio);
  Transition* transition = &data->transition;

  if(transition->state != Transition::Inactive){
    transition->fade_time_elapsed += dt;
    if(transition->fade_time_elapsed >= transition->fade_time_duration){
      transition->fade_time_elapsed = 0;
      switch(transition->state){
        case Transition::Inactive:
          break;
        case Transition::FadeTo:
          transition->state = Transition::FadeFrom;

          if(data->level_advance_pending){
            data->currentLevelIndex = data->next_level;
            data->level_advance_pending = false;
            StartLevel(data);
          }
          break;
        case Transition::FadeFrom:
          transition->state = Transition::Inactive;
          break;
      }
    }
  }

  switch(data->scene_current){
    case SCENE_TYPES::TITLESCREEN:
      UpdateMenu(&data->mainMenu, data);
      break;
    case SCENE_TYPES::GAME:
      UpdateGame(data, dt);
      break;
    case SCENE_TYPES::NONE:
      assert(false);
      break;
  }
}

void Draw(GameData* data, SDL_Renderer* renderer){
  DEV::PreDraw(data->imGui_context);
  SDL_SetRenderDrawColor(renderer, 120, 70, 120, 255);
  SDL_RenderClear(renderer);

  Sprite* black = GetSprite(SPRITE_ID::Black_1x1, data->spriteBuffer);
  switch(data->transition.state){
    case Transition::Inactive:
      DrawScene(data, data->scene_current, renderer);
      break;
    case Transition::FadeTo:{
      DrawScene(data, data->scene_previous, renderer);
      float alpha = data->transition.fade_time_elapsed / data->transition.fade_time_duration;
      SDL_SetRenderViewport(renderer, nullptr);
      SDL_SetRenderScale(renderer, 1, 1);
      RenderSprite_World(black, renderer, &data->camera, 0, 0, SCREEN_WIDTH, alpha);
      break;
    }
    case Transition::FadeFrom:{
      DrawScene(data, data->scene_current, renderer);
      float alpha = 1 - data->transition.fade_time_elapsed / data->transition.fade_time_duration;
      SDL_SetRenderViewport(renderer, nullptr);
      SDL_SetRenderScale(renderer, 1, 1);
      RenderSprite_World(black, renderer, &data->camera, 0, 0, SCREEN_WIDTH, alpha);
      break;
    }
  }

  SDL_SetRenderScale(renderer, 1, 1);
  SDL_SetRenderViewport(renderer, nullptr);
  DEV::Draw(data, renderer);
  SDL_RenderPresent(renderer);
}

void OnQuit(SDL_Renderer* renderer){
  SDL_DestroyRenderer(renderer);
}
}
