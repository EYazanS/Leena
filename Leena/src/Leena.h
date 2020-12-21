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

struct TileChunk
{
	uint32* Tiles;
};

struct World
{
	uint32 ChunkDimension;

	real32 TileSideInMeters;
	
	int32 TileSideInPixels;

	real32 MetersToPixels;
	
	uint32 TileChunkCountX;
	uint32 TileChunkCountY;
	
	uint32 ChunkShift;
	uint32 ChunkMask;

	TileChunk* TileChunks;
};

#define LeenaH

#endif