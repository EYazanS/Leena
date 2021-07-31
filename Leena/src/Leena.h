#pragma once
#pragma once

#if !defined(LeenaH)

#include "GameTypes.h"
#include "Math/Math.h"
#include "GameStructs.h"
#include "RandomNumbers.h"
#include "Memory/Memory.h"
#include "Utilities/Intrinsics.h"
#include "World/World.h"

// Functions provided for the platform layer
DllExport void GameUpdateAndRender(ThreadContext* thread, GameMemory* gameMemory, ScreenBuffer* gameScreenBuffer, GameInput* input);
DllExport void GameUpdateAudio(ThreadContext* thread, GameMemory* gameMemory, AudioBuffer* audioBuffer);

#define Minimum(a, b) ((a < b) ? a : b)
#define Maximum(a, b) ((a > b) ? a : b)

#define HitPointMaxAmount 4

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

	V2 Align;
};

enum class EntityType
{
	Null,
	Player,
	Wall,
	Familiar,
	Monster
};

struct Hitpoint
{
	u8 Flags;
	u32 Current;
};

struct HighEntity
{
	V2 Position;
	V2 Velocity;
	i32 PositionZ;

	u32 FacingDirection;

	r32 Z;
	r32 dZ;

	u32 LowEntityIndex;
};

struct LowEntity
{
	EntityType Type;
	WorldPosition Position;
	r32 Width, Height;
	r32 Speed;
	b32 Collides;

	// This is for vertical change, aka stairs
	i32 TileZ;

	u32 HighEntityIndex;

	u32 MaxHp;
	Hitpoint Hitpoints[16];
};

struct AddLowEntityResult
{
	u32 LowEntityIndex;
	LowEntity* LowEntity;
};

struct Entity
{
	u32 LowEntityIndex;
	LowEntity* Low;
	HighEntity* High;
};

struct EntityVisiblePiece
{
	LoadedBitmap* Bitmap;
	V2 Offset;
	r32 Z;
	r32 Alpha;
	r32 ZCoefficient;
	Colour Colour;
	V2 Dimensions;
};

struct GameState
{
	MemoryPool WorldMemoryPool;

	WorldPosition CameraPosition;

	u32 LowEntitiesCount;
	u32 HighEntitiesCount;

	r32 MetersToPixels;

	HighEntity HighEntities[256];
	LowEntity LowEntities[100000];

	Entity PlayerEntity;

	LoadedBitmap Background;
	PlayerBitMap BitMaps[4];

	LoadedBitmap Tree;
	LoadedBitmap Rock;

	World* World;
};

struct EntityVisiblePieceGroup
{
	u32 PieceCount;
	GameState* GameState;
	EntityVisiblePiece Pieces[32];
};

void MakeEntityLowFreq(GameState* gameState, u32 index);
HighEntity* MakeEntityHighFreq(GameState* gameState, u32 entityIndex);
HighEntity* MakeEntityHighFreq(GameState* gameState, LowEntity* lowEntity, u32 index, V2 camerSpaceP);

inline LowEntity* GetLowEntity(GameState* gameState, u32 index)
{
	LowEntity* result = nullptr;

	if (index > 0 && index < gameState->LowEntitiesCount)
	{
		result = gameState->LowEntities + index;
	}

	return result;
}

inline HighEntity* GetHighEntity(GameState* gameState, u32 index)
{
	HighEntity* result = nullptr;

	if (index > 0 && index < gameState->HighEntitiesCount)
	{
		result = gameState->HighEntities + index;
	}

	return result;
}

inline void OffsetAndCheckFrequencyByArea(GameState* gameState, V2 offset, R2 highFreqBounds)
{
	for (u32 highEntityIndex = 1; highEntityIndex < gameState->HighEntitiesCount; )
	{
		HighEntity* highEntity = gameState->HighEntities + highEntityIndex;

		highEntity->Position += offset;

		if (IsInRectangle(highFreqBounds, highEntity->Position))
		{
			highEntityIndex++;
		}
		else
		{
			MakeEntityLowFreq(gameState, highEntity->LowEntityIndex);
		}
	}
}

inline b32 ValidateEntityPairs(GameState* state)
{
	b32 valid = true;

	for (u32 highEntityIndex = 1; highEntityIndex < state->HighEntitiesCount; highEntityIndex++)
	{
		HighEntity* high = state->HighEntities + highEntityIndex;
		valid = valid && (state->LowEntities[high->LowEntityIndex].HighEntityIndex == highEntityIndex);
	}

	return valid;
}

#define LeenaH
#endif