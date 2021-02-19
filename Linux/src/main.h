#pragma once
#include <Leena.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <dlfcn.h>
#include <fstream>
#include <sys/mman.h>

#define GAME_UPDATE(name) void name(ThreadContext *thread, GameMemory *gameMemory, GameScreenBuffer *gameScreenBuffer, GameAudioBuffer *soundBuffer, GameInput *input)
typedef GAME_UPDATE(GAMEUPDATE);
GAME_UPDATE(GameUpdateStub){};
struct LinuxProgramState
{
	bool IsRunning;
	uint16 yes;
};

struct GameCode
{
	void *LibraryHandle;
	GAMEUPDATE *Update;
	bool IsValid;
};

SDL_Window *CreateWindow();
GameCode LinuxLoadGameCode();
GameMemory InitGameMemory();
void LinuxUnloadGameCode(GameCode *gameCode);

// Input
void LinuxProccessKeyboardMessage(GameButtonState &state, bool32 isPressed, int repeat);
void ProccessKeyboardKeys(LinuxProgramState *state, SDL_Event &event, KeyboardInput *controller);