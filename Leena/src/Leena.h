#pragma once

#if !defined(LeenaH)

#include "GameTypes.h"
#include "GameStructs.h"
#include "RandomNumbers.h"
#include "Memory/Memory.h"
#include "Utilities/Intrinsics.h"
#include "Map/TileMap.h"

// Functions provided for the platform layer
DllExport void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameInput* input);

inline GameControllerInput* GetController(GameInput* input, uint8 index)
{
	Assert(index < ArrayCount(input->Controllers));
	GameControllerInput* result = &input->Controllers[index];
	return result;
}

struct World
{
	Map* Map;
};

struct LoadedBitmap
{
	int64 Width;
	int64 Height;
	uint32* Data;
};

struct PlayerBitMap
{
	LoadedBitmap Head;
	LoadedBitmap Torso;
	LoadedBitmap Cape;

	int32 AlignX;
	int32 AlignY;
};

struct GameState
{
	MemoryPool WorldMemoryPool;
	World* World;
	MapPosition CameraPosition;
	MapPosition PlayerPosition;
	LoadedBitmap Background;
	uint32 PlayerFacingDirection;
	PlayerBitMap playerBitMaps[4];
	bool32 EnableSmoothCamera;
};
#define LeenaH

#endif