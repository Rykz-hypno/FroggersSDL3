#include "input.h"
#include <cstring>

bool KeyPressed(const Input* input, SDL_Scancode key){
  if(input->keys_previous == nullptr){
    return input->keys_current[key];
  }
  return input->keys_current[key] && !input->keys_previous[key];
}

bool KeyHeld(const Input* input, SDL_Scancode key){
  if(input->keys_previous == nullptr){
    return false;
  }
  return input->keys_current[key] && input->keys_previous[key];
}

bool KeyReleased(const Input* input, SDL_Scancode key){
  if(input->keys_previous == nullptr){
    return false;
  }
  return !input->keys_current[key] && input->keys_previous[key];
}

bool AnyKeyPressed(const Input* input){
  for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
    if(KeyPressed(input, (SDL_Scancode)i)){
      return true;
    }
  }
  return false;
}

bool KeyHeld_ForTime(const Input* input, SDL_Scancode key, float min_length){
  return input->keys_held_time[key] >= min_length;
}

void UpdateKeys(Input* input, float dt){
  for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
    if (input->keys_current[i]){
      input->keys_held_time[i] += dt;
    }
    else{
      input->keys_held_time[i] = 0;
    }
  }
  memcpy((void*)input->keys_previous, input->keys_current, SDL_SCANCODE_COUNT * sizeof(bool));
}

void ResetKeyHeldTime(Input* input, SDL_Scancode key){
  input->keys_held_time[key] = 0;
}

void ResetAll(Input* input){
  memset((void*)input->keys_current, 0, sizeof(bool) * SDL_SCANCODE_COUNT);
  memset((void*)input->keys_previous, 0, sizeof(bool) * SDL_SCANCODE_COUNT);
  memset((void*)input->keys_held_time, 0, sizeof(float) * SDL_SCANCODE_COUNT);
}

static SDL_MouseButtonFlags ButtonToFlag(MouseButtons button){
  switch(button){
    case MouseButtons::LEFT:
      return SDL_BUTTON_LMASK;
    case MouseButtons::MIDDLE:
      return SDL_BUTTON_MMASK;
    case MouseButtons::RIGHT:
      return SDL_BUTTON_RMASK;
  }
  return 0;
}

bool MousePressed(const Input* input, MouseButtons button){
  SDL_MouseButtonFlags flag = ButtonToFlag(button);
  return (input->mouse_current & flag) != 0 && (input->mouse_previous & flag) == 0;
}

bool MouseReleased(const Input* input, MouseButtons button){
  SDL_MouseButtonFlags flag = ButtonToFlag(button);
  return (input->mouse_current & flag) == 0 && (input->mouse_previous & flag) != 0;
}

bool MouseHeld(const Input* input, MouseButtons button){
  SDL_MouseButtonFlags flag = ButtonToFlag(button);
  return (input->mouse_current & flag) != 0 && (input->mouse_previous & flag) != 0;
}

bool MouseHeld_ForTime(const Input* input, MouseButtons button, float min_length){

  return input->mouse_held_time[(int)button] >= min_length;
}

void UpdateMouse(Input* input, float dt){
  const MouseButtons buttons[3] = {MouseButtons::LEFT, MouseButtons::MIDDLE, MouseButtons::RIGHT};
  for(MouseButtons b : buttons){

    if((input->mouse_current & ButtonToFlag(b)) != 0){
      input->mouse_held_time[(int)b] += dt;
    }
    else{
      input->mouse_held_time[(int)b] = 0;
    }
  }
  input->mouse_previous = input->mouse_current;
}
