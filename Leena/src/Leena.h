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
	LoadedBitmap Shadow;

	i32 AlignX;
	i32 AlignY;
};

enum class EntityResidence
{
	NoneExistent,
	Dormant,
	Low,
	High
};

enum class EntityType
{
	Null,
	Player,
	Wall
};

struct HighEntity
{
	V2 Position;
	V2 Velocity;
	i32 PositionZ;

	r32 Speed;
	u32 FacingDirection;

	r32 Z;
	r32 dZ;
};

struct LowEntity
{
};

struct DormantEntity
{
	MapPosition Position;
	EntityType Type;
	r32 Width, Height;
	b32 Collides;
	// This is for vertical change, aka stairs
	i32 TileZ;
};

struct Entity
{
	EntityResidence Residence;
	DormantEntity* Dormant;
	LowEntity* Low;
	HighEntity* High;
};

struct GameState
{
	MemoryPool WorldMemoryPool;

	MapPosition CameraPosition;

	u32 EntitiesCount;

	EntityResidence EntityResidence[256];
	HighEntity HighEntities[256];
	LowEntity LowEntities[256];
	DormantEntity DormantEntities[256];

	Entity PlayerEntity;

	LoadedBitmap Background;
	PlayerBitMap BitMaps[4];

	World* World;
};

void ChangeEntityResidence(GameState* gameState, u32 entityIndex, EntityResidence state);

inline EntityResidence GetCurrentEntityState(GameState* gameState, u32 index)
{
	EntityResidence reuslt = EntityResidence::NoneExistent;
	reuslt = gameState->EntityResidence[index];
	return reuslt;
}

inline Entity GetEntity(GameState* gameState, u32 index, EntityResidence state)
{
	Entity result = {};

	if (index > 0 && (index < gameState->EntitiesCount))
	{
		if (GetCurrentEntityState(gameState, index) < state)
		{
			ChangeEntityResidence(gameState, index, state);
			Assert(gameState->EntityResidence[index] >= state);
		}

		result.Residence = state;
		result.Dormant = &gameState->DormantEntities[index];
		result.Low = &gameState->LowEntities[index];
		result.High = &gameState->HighEntities[index];
	}

	return result;
}

#define LeenaH
#endif