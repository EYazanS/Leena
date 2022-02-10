#pragma once

#if !defined(LeenaH)

#include "GameStructs.h"

struct MemoryPool
{
	MemorySizeIndex Size;
	MemorySizeIndex UsedAmount;
	u8* BaseMemory;
};

// Functions provided for the platform layer
void initializePool(MemoryPool* pool, MemorySizeIndex size, u8* storage)
{
	pool->Size = size;
	pool->BaseMemory = storage;
	pool->UsedAmount = 0;
}

#define PushArray(pool, size, type) (type*) PushSize_(pool, size +  sizeof(type))
#define PushStruct(pool, type) (type*) PushSize_(pool, sizeof(type))

void* PushSize_(MemoryPool* pool, MemorySizeIndex size)
{
	Assert(pool->UsedAmount + pool->UsedAmount <= pool->Size);
	void* result = pool->BaseMemory + pool->UsedAmount;
	pool->UsedAmount += size;
	return result;
}

#define Minimum(a, b) ((a < b) ? a : b)
#define Maximum(a, b) ((a > b) ? a : b)

#define HitPointMaxAmount 4

#include "Intrinsics.h"
#include "LeenaMath.h"
#include "World/World.h"

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
	Monster,
	Sword
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

	u32 SwordLowIndex;

	r32 Timer;

	r32 DistanceRemaining;
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
	r32 ZCoefficient;
	V4 Colour;
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

	LoadedBitmap Sword;

	World* World;
};

struct EntityVisiblePieceGroup
{
	u32 PieceCount;
	GameState* GameState;
	EntityVisiblePiece Pieces[32];
};
#define LeenaH
#endif