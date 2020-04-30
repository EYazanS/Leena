#pragma once

#define GAME_UPDATE(name) void name(GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
typedef GAME_UPDATE(game_update);
GAME_UPDATE(GameUpdateStub) {};

struct GameCode
{
	HMODULE LibraryHandle;
	game_update* Update;
	bool32 IsValid;
	FILETIME LastWriteTime;
};

// Game
internal GameCode Win32LoadGameCode();
internal void Win32UnloadGameCode(GameCode* gameCode);