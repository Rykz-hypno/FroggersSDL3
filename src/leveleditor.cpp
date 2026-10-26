#include "leveleditor.h"
#include "imgui/imgui.h"
#include "rendering.h"

namespace EDITOR {

static void ObjectButton(const char* name, ID id, Editor* editor, Sprite* spriteBuffer){
  ImVec2 size = {32, 32};
  if(ImGui::ImageButton(name, (ImTextureID)GetSpriteFromID(id, spriteBuffer)->texture, size)){
    editor->object_to_place_id = id;
  }
}

void DrawObjectPanel(Editor* editor, Sprite* spriteBuffer){
  ImGui::Begin("objects");

  ObjectButton("Ground", ID::GROUND, editor, spriteBuffer);
  ImGui::SameLine();
  ObjectButton("Wall", ID::WALL, editor, spriteBuffer);
  ImGui::SameLine();
  ObjectButton("Goal", ID::GOAL, editor, spriteBuffer);
  ImGui::SameLine();
  ObjectButton("Box", ID::BOX, editor, spriteBuffer);
  ImGui::SameLine();
  ObjectButton("Player", ID::PLAYER, editor, spriteBuffer);

  ImGui::Text("Vanster musknapp: placera");
  ImGui::Text("Hoger musknapp: ta bort entity");

  ImGui::End();
}

void PlaceObject(const int x, const int y, Editor* editor, LevelData* level, CommandBuffer* buffer, uint32_t* timestamp){
  ID id = editor->object_to_place_id;
  if(id == ID::GROUND || id == ID::WALL || id == ID::GOAL){

    level->cells[y * level->w + x] = (uint8_t)id;
    return;
  }

  Entity* existing = level->GetEntity(x, y);
  if(existing != nullptr && existing->id == id){
    return;
  }

  (*timestamp)++;
  if(existing != nullptr){
    RemoveCommand remove(existing, level);
    Push(buffer, remove, level, *timestamp);
  }
  AddCommand add(x, y, id);
  Push(buffer, add, level, *timestamp);
}

void DrawPreview(Editor* editor, Input* input, SDL_Renderer* renderer, LevelData* level, Camera* camera, Sprite* spriteBuffer){

  if(ImGui::GetIO().WantCaptureMouse){
    return;
  }
  if(camera::GetIsPointInsideGrid(input->mouse_x, input->mouse_y, level) == false){
    return;
  }

  int x;
  int y;
  camera::WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
  Sprite* preview = GetSpriteFromID(editor->object_to_place_id, spriteBuffer);
  if(preview != nullptr){
    RenderSprite_Grid(preview, level, renderer, camera, x, y, 1, 0.5);
  }
}

void Update(Editor* editor, Input* input, LevelData* level, CommandBuffer* buffer, uint32_t* timestamp){

  if(ImGui::GetIO().WantCaptureMouse){
    return;
  }

  if(MousePressed(input, MouseButtons::LEFT)){
    if(camera::GetIsPointInsideGrid(input->mouse_x, input->mouse_y, level)){
      int x;
      int y;
      camera::WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
      PlaceObject(x, y, editor, level, buffer, timestamp);
    }
  }
  else if(MousePressed(input, MouseButtons::RIGHT)){
    if(camera::GetIsPointInsideGrid(input->mouse_x, input->mouse_y, level)){
      int x;
      int y;
      camera::WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
      Entity* entity = level->GetEntity(x, y);
      if(entity == nullptr){
        return;
      }
      (*timestamp)++;
      RemoveCommand remove(entity, level);
      Push(buffer, remove, level, *timestamp);
    }
  }
}

}
