#pragma once
#include <cstdint>
#include "entity.h"

enum class CMD_TYPE : uint8_t {
  NONE = 0,
  MOVE = 1,
  ADD = 2,
  REMOVE = 3
};

struct LevelData;

struct Command {
  CMD_TYPE type;
  uint32_t timestamp;
};

struct MoveCommand : Command {
  Entity* entity;
  int xDir;
  int yDir;
};

struct AddCommand : Command {
  int x;
  int y;
  ID id;

  AddCommand(int x, int y, ID id){
    this->x = x;
    this->y = y;
    this->id = id;
    type = CMD_TYPE::ADD;
  }
};

struct RemoveCommand : Command {
  int x;
  int y;
  int slot_index;
  ID storedID;
  Behaviour storedBehaviour;

  RemoveCommand(Entity* entity, LevelData* level);
};

union AnyCommand {
  Command command;
  MoveCommand move;
  AddCommand add;
  RemoveCommand remove;

  AnyCommand(MoveCommand mv){
    move = mv;
  };
  AnyCommand(AddCommand a){
    add = a;
  };
  AnyCommand(RemoveCommand r){
    remove = r;
  };
};

struct CommandBuffer {
  AnyCommand* allCommands;
  int capacity;
  int index;
  int head;
};

void Push(CommandBuffer* buffer, AnyCommand cmd, LevelData* level, uint32_t timestamp);
void Undo(CommandBuffer* buffer, LevelData* level);
void Redo(CommandBuffer* buffer, LevelData* level);
