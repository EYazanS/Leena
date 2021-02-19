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