#include "audioSystem.h"
#include "FMOD/fmod.h"
#include "FMOD/fmod_errors.h"
#include "arena.h"
#include "common.h"
#include <cassert>
#include <cstdint>
#include <cstdio>

AudioSystem* g_audioSystem;

static const SoundDataEntry all_sound_data[] = {
  {SFX_ID::FALLBACK, "assets/audio/sfx/fallback.wav"},
  {SFX_ID::STEP,     "assets/audio/sfx/step.wav"},
  {SFX_ID::TONGUE,   "assets/audio/sfx/tongue.wav"},
  {SFX_ID::WIN,      "assets/audio/sfx/win.wav"},
  {SFX_ID::CLICK,    "assets/audio/sfx/click.wav"},
};

void InitializeAudioSystem(AudioSystem* audio, Memory::Arena* arena_audio){
  assert(audio->initialized == false);

  uintptr_t start = (uintptr_t)arena_audio->base;
  uintptr_t aligned_start = (start + 511) & ~(uintptr_t)511;
  int pool_size = (int)(arena_audio->size - (aligned_start - start));
  pool_size -= pool_size % 512;

  FMOD_RESULT memory_init_ok = FMOD_Memory_Initialize((void*)aligned_start, pool_size, nullptr, nullptr, nullptr, FMOD_MEMORY_ALL);
  if(memory_init_ok != FMOD_OK){
    printf("FMOD minne: %s\n", FMOD_ErrorString(memory_init_ok));
  }
  assert(memory_init_ok == FMOD_OK);

  FMOD_RESULT system_creation_ok = FMOD_System_Create(&audio->sound_system, FMOD_VERSION);
  assert(system_creation_ok == FMOD_OK);

  FMOD_RESULT system_init_ok = FMOD_System_Init(audio->sound_system, AudioSystem::CHANNEL_COUNT, FMOD_INIT_NORMAL, nullptr);
  assert(system_init_ok == FMOD_OK);

  g_audioSystem = audio;
  audio->initialized = true;
}

static const int NOT_FOUND = -1;
static const float MASTER_VOLUME = 0.2f;

static int GetAvailableChannelIndex(AudioSystem* audio){
  for(int i = 0; i < AudioSystem::CHANNEL_COUNT; i++){
    FMOD_CHANNEL* channel = audio->channels[i];
    if(channel == nullptr){
      return i;
    }
    FMOD_BOOL is_playing = false;
    FMOD_Channel_IsPlaying(channel, &is_playing);
    if(is_playing == false){
      return i;
    }
  }
  return NOT_FOUND;
}

void PlaySFX(SFX_ID id, float volume){
  if(g_audioSystem == nullptr || g_audioSystem->initialized == false){
    return;
  }
  assert(id != SFX_ID::COUNT);

  FMOD_SOUND* sfx = g_audioSystem->soundEffects[(int)id];
  if(sfx == nullptr){
    sfx = g_audioSystem->soundEffects[(int)SFX_ID::FALLBACK];
  }
  if(sfx == nullptr){
    return;
  }

  int channel_index = GetAvailableChannelIndex(g_audioSystem);
  if(channel_index == NOT_FOUND){
    return;
  }
  FMOD_CHANNEL** channel_slot = &g_audioSystem->channels[channel_index];

  FMOD_System_PlaySound(g_audioSystem->sound_system, sfx, nullptr, false, channel_slot);
  FMOD_Channel_SetVolume(*channel_slot, volume * MASTER_VOLUME);
}

void UpdateAudio(AudioSystem* audio){
  if(g_audioSystem != audio){
    g_audioSystem = audio;
  }
  assert(audio->initialized);
  FMOD_System_Update(audio->sound_system);
}

namespace AssetManagement{
  void LoadAllSFX(AudioSystem* audioSystem){
    for(const SoundDataEntry& sound_data : all_sound_data){
      FMOD_RESULT result = FMOD_System_CreateSound(audioSystem->sound_system, sound_data.path, FMOD_DEFAULT, nullptr,
                                                   &audioSystem->soundEffects[(int)sound_data.id]);
      if(result != FMOD_OK){
        audioSystem->soundEffects[(int)sound_data.id] = nullptr;
        printf("saknar ljudfil: %s\n", sound_data.path);
      }
    }
  }
}
