#pragma once

#if !defined(LeenaH)

#include "GameTypes.h"
#include "GameStructs.h"
#include "Utilities\Intrinsics.h"

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
	WorldPosition PlayerPosition;
};

struct TileMap
{
	uint32* Tiles;
};

struct World
{
	uint32 CountX;
	uint32 CountY;

	real32 TileSideInMeters;

	int32 LowerLeftX;
	int32 LowerLeftY;
	
	int32 TileSideInPixels;

	real32 MetersToPixels;
	
	int32 TileMapCountX;
	int32 TileMapCountY;
	
	TileMap* TileMaps;
};

#define LeenaH

#endif