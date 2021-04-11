#pragma once
#pragma once

#if !defined(LeenaH)

#include "GameTypes.h"
#include "Math/Math.h"
#include "GameStructs.h"
#include "RandomNumbers.h"
#include "Memory/Memory.h"
#include "Utilities/Intrinsics.h"
#include "Map/TileMap.h"

// Functions provided for the platform layer
DllExport void GameUpdateAndRender(ThreadContext* thread, GameMemory* gameMemory, ScreenBuffer* gameScreenBuffer, GameInput* input);
DllExport void GameUpdateAudio(ThreadContext* thread, GameMemory* gameMemory, AudioBuffer* audioBuffer);

#define Minimum(a, b) ((a < b) ? a : b)
#define Maximum(a, b) ((a > b) ? a : b)

struct World
{
	Map* Map;
};

struct LoadedBitmap
{
	i64 Width;
	i64 Height;
	u32* Data;
};

struct PlayerBitMap
{
	LoadedBitmap Head;
	LoadedBitmap Torso;
	LoadedBitmap Cape;

	i32 AlignX;
	i32 AlignY;
};

struct Entity
{
	b32 Exists;

	r32 Width;
	r32 Height;

	MapPosition Position;
	V2 Velocity;
	
	r32 Speed;

	u32 FacingDirection;
};

struct GameState
{
	MemoryPool WorldMemoryPool;

	MapPosition CameraPosition;

	u32 EntitiesCount;

	Entity Entities[250];

	Entity PlayerEntity;

	LoadedBitmap Background;
	PlayerBitMap BitMaps[4];

	World* World;
};

inline Entity* GetEntity(GameState* gameState, u32 index)
{
	Entity* result = {};

	if (index > 0 && (index < ArrayCount(gameState->Entities)))
	{
		result = &gameState->Entities[index];
	}
	return result;
}

#define LeenaH
#endif