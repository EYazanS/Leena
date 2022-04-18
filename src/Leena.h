#pragma once

#if !defined(LeenaH)

#include "GameStructs.h"

#define LocalPersist static
#define GlobalVariable static
#define internal static

struct MemoryPool
{
	MemorySizeIndex Size;
	MemorySizeIndex UsedAmount;
	u8 *BaseMemory;
};

// Functions provided for the platform layer
inline void initializePool(MemoryPool *pool, MemorySizeIndex size, void *storage)
{
	pool->Size = size;
	pool->BaseMemory = (u8 *)storage;
	pool->UsedAmount = 0;
}

#define PushArray(pool, size, type) (type *)PushSize_(pool, size + sizeof(type))
#define PushStruct(pool, type) (type *)PushSize_(pool, sizeof(type))

inline void *PushSize_(MemoryPool *pool, MemorySizeIndex size)
{
	Assert(pool->UsedAmount + pool->UsedAmount <= pool->Size);
	void *result = pool->BaseMemory + pool->UsedAmount;
	pool->UsedAmount += size;
	return result;
}

#define ZeroStruct(instance) ZeroSize(sizeof(instance), &(instance))

inline void ZeroSize(MemorySizeIndex size, void *ptr)
{
	u8 *byte = (u8 *)ptr;

	while (size--)
	{
		*byte++ = 0;
	}
}

#define Minimum(a, b) ((a < b) ? a : b)
#define Maximum(a, b) ((a > b) ? a : b)

#include "Intrinsics.h"
#include "LeenaMath.h"
#include "World/World.h"
#include "SimRegion/SimRegion.h"
#include "Entities/Entities.h"

struct LoadedBitmap
{
	i64 Width;
	i64 Height;
	u32 *Data;
};

struct PlayerBitMap
{
	LoadedBitmap Head;
	LoadedBitmap Torso;
	LoadedBitmap Cape;
	LoadedBitmap Shadow;

	V2 Align;
};

struct LowEntity
{
	SimEntity Entity;
	WorldPosition Position;
};

struct EntityVisiblePiece
{
	LoadedBitmap *Bitmap;
	V2 Offset;
	r32 Z;
	r32 ZCoefficient;
	V4 Colour;
	V2 Dimensions;
};

struct ControlRequest
{
	V2 Acceleration;
	V2 SwordAcceleration;
	r32 Dz;
};

struct GameState
{
	MemoryPool WorldMemoryPool;

	WorldPosition CameraPosition;

	u32 LowEntitiesCount;

	r32 MetersToPixels;

	LowEntity LowEntities[100000];

	SimEntity *PlayerEntity;

	LoadedBitmap Background;
	PlayerBitMap BitMaps[4];

	LoadedBitmap Tree;
	LoadedBitmap Rock;

	LoadedBitmap Sword;

	World *World;
};

struct EntityVisiblePieceGroup
{
	u32 PieceCount;
	GameState *GameState;
	EntityVisiblePiece Pieces[32];
};

inline LowEntity *GetLowEntity(GameState *gameState, u32 index)
{
	LowEntity *result = 0;

	if (index > 0 && (index < gameState->LowEntitiesCount))
	{
		result = gameState->LowEntities + index;
	}

	return result;
}

inline MoveSpec GetDefaultMoveSpec()
{
	MoveSpec result = {};

	result.Drag = 0.0f;
	result.Speed = 1.0f;
	result.UnitMaxAccVector = false;

	return result;
}
#define LeenaH
#endif