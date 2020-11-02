#pragma once

#if !defined(LeenaH)

#include "GameTypes.h"
#include "GameStructs.h"

#define DllExport extern "C" __declspec(dllexport)

// Functions provided for the platform layer
DllExport void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameAudioBuffer* soundBuffer, GameInput* input);

inline GameControllerInput* GetController(GameInput* input, uint8 index)
{
	Assert(index < ArrayCount(input->Controllers));
	GameControllerInput* result = &input->Controllers[index];
	return result;
}

struct GameState
{
	int PlayerTileMapX;
	int PlayerTileMapY;

	int PlayerX;
	int PlayerY;
};

struct TileMap
{
	uint32* Tiles;
};

struct World
{
	int32 CountX;
	int32 CountY;
	int32 UpperLeftX;
	int32 UpperLeftY;
	int32 TileWidth;
	int32 TileHeight;
	int32 TileMapCountX;
	int32 TileMapCountY;
	TileMap* TileMaps;
};

#define LeenaH

#endif