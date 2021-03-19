#pragma once

#define GAME_UPDATE(name) void name(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameInput* input)
typedef GAME_UPDATE(GAMEUPDATE);
GAME_UPDATE(GameUpdateStub) {};

struct GameCode
{
	HMODULE LibraryHandle;
	GAMEUPDATE* UpdateAndRender;
	bool32 IsValid;
	FILETIME LastWriteTime;
};

// Game
internal GameCode Win32LoadGameCode(char* sourceDLLName, char* tempDLLName);
internal void Win32UnloadGameCode(GameCode* gameCode);