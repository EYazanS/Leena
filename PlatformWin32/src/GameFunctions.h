#pragma once

#define GAME_UPDATE(name) void name(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
typedef GAME_UPDATE(GAMEUPDATE);
GAME_UPDATE(GameUpdateStub) {};

struct GameCode
{
	HMODULE LibraryHandle;
	GAMEUPDATE* Update;
	bool32 IsValid;
	FILETIME LastWriteTime;
};

// Game
internal GameCode Win32LoadGameCode();
internal void Win32UnloadGameCode(GameCode* gameCode);