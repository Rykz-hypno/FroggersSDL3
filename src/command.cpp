#include "command.h"
#include "levels.h"

RemoveCommand::RemoveCommand(Entity* entity, LevelData* level){
  x = entity->x;
  y = entity->y;
  slot_index = (int)(entity - level->entityBuffer);
  storedID = entity->id;
  storedBehaviour = entity->behaviour;
  type = CMD_TYPE::REMOVE;
}

static void Execute(AnyCommand cmd, LevelData* level, bool from_redo = false){
  switch(cmd.command.type){
    case CMD_TYPE::NONE:
      break;
    case CMD_TYPE::MOVE:{
      MoveCommand mv = cmd.move;
      mv.entity->x_prev = mv.entity->x;
      mv.entity->y_prev = mv.entity->y;
      mv.entity->x += mv.xDir;
      mv.entity->y += mv.yDir;
      if(from_redo){
        mv.entity->progress_01 = 1;
      }
      break;
    }
    case CMD_TYPE::ADD:{
      AddCommand* add = &cmd.add;
      AddEntity(add->id, add->x, add->y, level);
      break;
    }
    case CMD_TYPE::REMOVE:{
      RemoveCommand* remove = &cmd.remove;
      RemoveEntity(remove->x, remove->y, level);
      break;
    }
  }
}

void Push(CommandBuffer* buffer, AnyCommand cmd, LevelData* level, uint32_t timestamp){
  if(buffer->index >= buffer->capacity){
    return;
  }
  buffer->allCommands[buffer->index] = cmd;
  buffer->allCommands[buffer->index].command.timestamp = timestamp;
  buffer->index++;
  buffer->head = buffer->index;
  Execute(cmd, level);
}

void Undo(CommandBuffer* buffer, LevelData* level){
  if(buffer->index == 0){
    return;
  }
  buffer->index--;

  AnyCommand cmd = buffer->allCommands[buffer->index];
  uint32_t timestamp = cmd.command.timestamp;
  switch(cmd.command.type){
    case CMD_TYPE::NONE:
      break;
    case CMD_TYPE::MOVE:{
      MoveCommand mv = cmd.move;
      mv.entity->x -= mv.xDir;
      mv.entity->y -= mv.yDir;
      mv.entity->progress_01 = 1;
      break;
    }
    case CMD_TYPE::ADD:{
      AddCommand* add = &cmd.add;
      RemoveEntity(add->x, add->y, level);
      break;
    }
    case CMD_TYPE::REMOVE:{
      RemoveCommand* remove = &cmd.remove;

      Entity* entity = &level->entityBuffer[remove->slot_index];
      AddEntityInSlot(entity, remove->storedID, remove->x, remove->y);
      entity->behaviour = remove->storedBehaviour;
      break;
    }
  }

  if(buffer->index > 0){
    if(buffer->allCommands[buffer->index - 1].command.timestamp == timestamp){
      Undo(buffer, level);
    }
  }
}

void Redo(CommandBuffer* buffer, LevelData* level){
  if(buffer->index == buffer->head){
    return;
  }
  AnyCommand cmd = buffer->allCommands[buffer->index];
  if(cmd.command.type == CMD_TYPE::NONE){
    return;
  }
  buffer->index++;
  Execute(cmd, level, true);

  uint32_t timestamp = cmd.command.timestamp;
  if(buffer->index != buffer->head){
    AnyCommand nextCommand = buffer->allCommands[buffer->index];
    if(nextCommand.command.timestamp == timestamp){
      Redo(buffer, level);
    }
  }
}
