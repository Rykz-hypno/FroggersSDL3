#pragma once

#define KILOBYTES(n) ((size_t)n * 1024)
#define MEGABYTES(n) (KILOBYTES(n) * 1024)
#define GIGABYTES(n) (MEGABYTES(n) * 1024)

constexpr size_t GAME_MEMORY_ALLOWANCE = MEGABYTES(14);
constexpr size_t AUDIO_MEMORY_ALLOWANCE = MEGABYTES(5);

constexpr int FPS = 240;
const double FRAME_TIME_MS = 1000.0 / FPS;

const int SCREEN_WIDTH = 650;
const int SCREEN_HEIGHT = 400;

const float WINDOW_SCALE = 2.0f;
const float CONTENT_SCALE = 1.0f;
const float WINDOW_WIDTH_PX = SCREEN_WIDTH * WINDOW_SCALE;
const float WINDOW_HEIGHT_PX = SCREEN_HEIGHT * WINDOW_SCALE;
const float CONTENT_OFFSET_X = (WINDOW_WIDTH_PX - SCREEN_WIDTH * CONTENT_SCALE) / 2.0f;
const float CONTENT_OFFSET_Y = (WINDOW_HEIGHT_PX - SCREEN_HEIGHT * CONTENT_SCALE) / 2.0f;
const int UPSCALE_FACTOR = 2;
const int CELL_SIZE_PX = 32 * UPSCALE_FACTOR;

const float MOVE_SPEED = 6.0;
const int TONGUE_RANGE = 2;
const float TONGUE_MISS_TIME = 0.2f;
const float TONGUE_COOLDOWN = 0.3f;
const float UNDO_REPEAT_TIME = 0.15;

const float HOP_HEIGHT_PX = 10.0f;

static const char STOP_CHAR = '\0';
inline bool IsStringEmpty(const char* str){
  return str == nullptr || str[0] == STOP_CHAR;
}
