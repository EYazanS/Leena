#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include <dlfcn.h>
#include <fstream>

#define GAME_UPDATE(name) void name()
typedef GAME_UPDATE(GAMEUPDATE);
GAME_UPDATE(GameUpdateStub) {};

struct LinuxProgramState
{
    bool IsRunning;
};

struct GameCode
{
	void* LibraryHandle;
	GAMEUPDATE* Update;
	bool IsValid;
};
