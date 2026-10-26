#include <cstdlib>
#include <windows.h>
#include <fileapi.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_timer.h"
#include "SDL_TTF/SDL_ttf.h"
#include "common.h"
#include "arena.h"
#include "gamestate.h"
#include "input.h"

SDL_Window* window;
SDL_Renderer* renderer;

Uint64 NOW = 0;
Uint64 PREV = 0;

constexpr const char* NAME_OF_DLL = "FROGGERS_game.dll";
constexpr const char* NAME_OF_TEMP_DLL = "FROGGERS_temp.dll";

typedef void (*Function_Initialize) (GameData* data, SDL_Window* window, SDL_Renderer* renderer);
typedef bool (*Function_HandleEvents) (GameData* data, SDL_Event event);
typedef void (*Function_Update) (GameData* data, float dt);
typedef void (*Function_Draw) (GameData* data, SDL_Renderer* renderer);
typedef void (*Function_OnQuit) (SDL_Renderer* renderer);

constexpr const char* NAME_OF_FUNC_INIT = "Initialize";
constexpr const char* NAME_OF_FUNC_HANDLE_EVENT = "HandleEvents";
constexpr const char* NAME_OF_FUNC_UPDATE = "Update";
constexpr const char* NAME_OF_FUNC_DRAW = "Draw";
constexpr const char* NAME_OF_FUNC_QUIT = "OnQuit";

struct DLL_INFO{
    HMODULE dll;
    FILETIME timestamp;
    Function_Initialize initialize;
    Function_HandleEvents handleEvents;
    Function_Update update;
    Function_Draw draw;
    Function_OnQuit quit;
};

FILETIME GetTimestamp(){
    WIN32_FIND_DATA data;
    HANDLE handle = FindFirstFile(NAME_OF_DLL, &data);
    FILETIME time_of_last_change = data.ftLastWriteTime;
    FindClose(handle);
    return time_of_last_change;
}

bool LoadDLL(DLL_INFO* info, int depth = 0){
    if(depth > 20){
        printf("failed to write temp DLL");
        return false;
    }
    bool success = CopyFile(NAME_OF_DLL, NAME_OF_TEMP_DLL, false);

    if(!success){
        Sleep(50);
        return LoadDLL(info, depth + 1);
    }

    info->dll = LoadLibrary(NAME_OF_TEMP_DLL);

    if(info->dll == nullptr){
        printf("could not load dll");
        return false;
    }

    info->initialize = (Function_Initialize)GetProcAddress(info->dll, NAME_OF_FUNC_INIT);
    info->handleEvents = (Function_HandleEvents)GetProcAddress(info->dll, NAME_OF_FUNC_HANDLE_EVENT);
    info->update = (Function_Update)GetProcAddress(info->dll, NAME_OF_FUNC_UPDATE);
    info->draw = (Function_Draw)GetProcAddress(info->dll, NAME_OF_FUNC_DRAW);
    info->quit = (Function_OnQuit)GetProcAddress(info->dll, NAME_OF_FUNC_QUIT);

    info->timestamp = GetTimestamp();

    return true;
}

void UnloadDLL(DLL_INFO* info){
    FreeLibrary(info->dll);
    info->dll = nullptr;
    DeleteFile(NAME_OF_TEMP_DLL);
}

void* AllocateGameMemory(){
    void* blob = malloc(GAME_MEMORY_ALLOWANCE);
    if(blob == nullptr){
        printf("fatal error: could not allocate memory");
        return nullptr;
    }
    return blob;
}

void SDL_Setup(){
    SDL_Init(SDL_INIT_EVENTS);
    TTF_Init();
    window = SDL_CreateWindow("FROGGERS", (int)WINDOW_WIDTH_PX, (int)WINDOW_HEIGHT_PX, 0);
    renderer = SDL_CreateRenderer(window, NULL);
}

void CalculateDeltaTime(float& dt, float scaler){
    NOW = SDL_GetTicksNS();
    dt = NOW - PREV;
    dt = SDL_NS_TO_SECONDS(dt);
    dt *= scaler;
    PREV = NOW;
}

void DLL_CheckStatus(DLL_INFO* dll){
    FILETIME timestamp = GetTimestamp();
    bool is_timestamp_changed = CompareFileTime(&dll->timestamp, &timestamp) != 0;
    if(is_timestamp_changed){
        UnloadDLL(dll);
        LoadDLL(dll);
    }
}

void CalculateRemainingFrameTime_MS(double* milliseconds){
    Uint64 frame_end_time_ns = SDL_GetTicksNS();
    double frame_time_spent_ns = frame_end_time_ns - PREV;
    double frame_time_spent_ms = frame_time_spent_ns / 1e6;
    *milliseconds = FRAME_TIME_MS - frame_time_spent_ms;
}

int main() {

    void* game_memory = AllocateGameMemory();
    if(game_memory == nullptr){
        return 1;
    }

    Memory::Arena* arena_main = new Memory::Arena();
    Memory::Initialize(arena_main, game_memory, GAME_MEMORY_ALLOWANCE);

    GameData* gameData = (GameData*)Memory::Allocate(arena_main, sizeof(GameData));

    int SPRITE_COUNT = 256;
    size_t IMAGE_ARENA_SIZE = sizeof(Sprite) * SPRITE_COUNT;
    gameData->arena_images = Memory::CreateSubArena(arena_main, IMAGE_ARENA_SIZE);
    gameData->spriteBuffer = (Sprite*)Memory::Allocate(gameData->arena_images, sizeof(Sprite) * SPRITE_COUNT);
    gameData->arena_levels = Memory::CreateSubArena(arena_main, MEGABYTES(3));
    gameData->arena_entities = Memory::CreateSubArena(gameData->arena_levels, MEGABYTES(1));
    gameData->arena_scratch = Memory::CreateSubArena(arena_main, KILOBYTES(256));
    gameData->arena_audio = Memory::CreateSubArena(arena_main, AUDIO_MEMORY_ALLOWANCE);
    gameData->arena_commands = Memory::CreateSubArena(gameData->arena_levels, MEGABYTES(1));

    size_t INPUT_ARENA_SIZE = 0;
    INPUT_ARENA_SIZE += sizeof(bool) * SDL_SCANCODE_COUNT * 2;
    INPUT_ARENA_SIZE += sizeof(float) * SDL_SCANCODE_COUNT;
    INPUT_ARENA_SIZE += sizeof(float) * 3;
    INPUT_ARENA_SIZE += 128;
    gameData->arena_input = Memory::CreateSubArena(arena_main, INPUT_ARENA_SIZE);

    gameData->input.keys_current = (bool*)Memory::Allocate(gameData->arena_input, sizeof(bool) * SDL_SCANCODE_COUNT);
    gameData->input.keys_previous = (bool*)Memory::Allocate(gameData->arena_input, sizeof(bool) * SDL_SCANCODE_COUNT);
    gameData->input.keys_held_time = (float*)Memory::Allocate(gameData->arena_input, sizeof(float) * SDL_SCANCODE_COUNT);

    gameData->input.mouse_held_time = (float*)Memory::Allocate(gameData->arena_input, sizeof(float) * 3);

    gameData->levels = (LevelData*)Memory::Allocate(gameData->arena_levels, sizeof(LevelData) * 12);

    gameData->input_buffer_capacity = 50;
    size_t RING_BUFFER_SIZE = sizeof(Position) * gameData->input_buffer_capacity;
    gameData->input_buffer = (Position*)Memory::Allocate(gameData->arena_levels, RING_BUFFER_SIZE);

    gameData->commandBuffer = (CommandBuffer*)Memory::Allocate(arena_main, sizeof(CommandBuffer));
    gameData->commandBuffer->capacity = 2000;
    size_t COMMAND_SIZE = sizeof(AnyCommand) * gameData->commandBuffer->capacity;
    gameData->commandBuffer->allCommands = (AnyCommand*)Memory::Allocate(gameData->arena_commands, COMMAND_SIZE);

    SDL_Setup();

    DLL_INFO dll;
    bool dll_successfully_loaded = LoadDLL(&dll);

    if(dll_successfully_loaded == false){
        return 2;
    }

    dll.initialize(gameData, window, renderer);

    MMRESULT result = timeBeginPeriod(1);
    if(result == TIMERR_NOCANDO){
        printf("could not increase timer resolution");
        Sleep(2000);
        return 3;
    }

    gameData->running = true;
    float dt;
    float dt_scaler = 1;
    gameData->dt = &dt;
    gameData->dt_scaler = &dt_scaler;
    while(gameData->running){

        DLL_CheckStatus(&dll);

        Memory::Reset(gameData->arena_scratch);

        CalculateDeltaTime(dt, dt_scaler);

        SDL_Event event;

        while(SDL_PollEvent(&event)){
            gameData->running = dll.handleEvents(gameData, event);
            if(gameData->running == false){
                break;
            }
        }

        gameData->input.keys_current = SDL_GetKeyboardState(nullptr);
        gameData->input.mouse_current = SDL_GetMouseState(&gameData->input.mouse_x, &gameData->input.mouse_y);

        bool in_menu = gameData->scene_current == SCENE_TYPES::TITLESCREEN;
        float mouse_scale = in_menu ? WINDOW_SCALE : CONTENT_SCALE;
        float mouse_offset_x = in_menu ? 0.0f : CONTENT_OFFSET_X;
        float mouse_offset_y = in_menu ? 0.0f : CONTENT_OFFSET_Y;
        gameData->input.mouse_x = (gameData->input.mouse_x - mouse_offset_x) / mouse_scale;
        gameData->input.mouse_y = (gameData->input.mouse_y - mouse_offset_y) / mouse_scale;
        dll.update(gameData, dt);
        UpdateKeys(&gameData->input, dt);
        UpdateMouse(&gameData->input, dt);

        dll.draw(gameData, renderer);

        double time_to_sleep_ms;
        CalculateRemainingFrameTime_MS(&time_to_sleep_ms);

        if(time_to_sleep_ms > 0){
            if(time_to_sleep_ms > 1){
                SDL_Delay(time_to_sleep_ms - 1);
            }
            while (time_to_sleep_ms > 0) {
                CalculateRemainingFrameTime_MS(&time_to_sleep_ms);
            }
        }
    }

    dll.quit(renderer);
    SDL_Quit();
    return 0;
}
