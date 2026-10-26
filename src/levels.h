#pragma once
#include "arena.h"
#include "entity.h"
#include <cstdint>
using namespace Memory;

const int MAX_ENTITIES = 256;

struct LevelData{
  int w;
  int h;
  uint8_t* cells;
  const char* level_path;
  Entity* entityBuffer;
  int entityCount;

  uint8_t GetCellID(int x, int y){
    return cells[y * w + x];
  }

Entity* GetEntity(int x, int y){
  for (int i = 0; i < entityCount; i++) {
    if(entityBuffer[i].id != ID::NONE && entityBuffer[i].x == x && entityBuffer[i].y == y){
      return &entityBuffer[i];
    }
  }

  return nullptr;
}

};

inline bool IsWalkable(uint8_t tile){
  return tile == (uint8_t)ID::GROUND || tile == (uint8_t)ID::GOAL;
}

void CreateLevel(Arena* arena, LevelData* level, const char* level_name);
void CreateEntities(LevelData* lvl_data, Arena* arena);
Entity* GetNextAvailableEntity(LevelData* level);
void AddEntity(ID entity_id, int x, int y, LevelData* level);
void AddEntityInSlot(Entity* slot, ID entity_id, int x, int y);
void RemoveEntity(int x, int y, LevelData* level);

