#pragma once

#define GAME_UPDATE_AND_RENDER(name) void name(ThreadContext* thread, GameMemory* gameMemory, ScreenBuffer* screenBuffer, GameInput* input)
typedef GAME_UPDATE_AND_RENDER(GAMEUPDATEANDRENDER);
GAME_UPDATE_AND_RENDER(GameUpdatAndRendereStub) {};

#define GAME_UPDATEAUDIO(name) void name(ThreadContext* thread, GameMemory* gameMemory, AudioBuffer* audioBuffer)
typedef GAME_UPDATEAUDIO(GAMEUPDATEAUDIO);
GAME_UPDATEAUDIO(GameUpdateAudioStub) {};

struct GameCode
{
	HMODULE LibraryHandle;
	GAMEUPDATEANDRENDER* UpdateAndRender;
	GAMEUPDATEAUDIO* UpdateAudio;
	b32 IsValid;
	FILETIME LastWriteTime;
};

// Game
internal GameCode Win32LoadGameCode(char* sourceDLLName, char* tempDLLName);
internal void Win32UnloadGameCode(GameCode* gameCode);