#pragma once

enum class EntityType
{
	Null,
	Player,
	Wall,
	Familiar,
	Monster,
	Sword
};

#define HitPointMaxAmount 4

struct Hitpoint
{
	u8 Flags;
	u32 Current;
};

struct SimEntity;

union EntityReference
{
	SimEntity *Ptr;
	u32 Index;
};

enum EntityFlag
{
	Collides = (1 << 1),
	Nonspatial = (1 << 2),
	Simming = (1 << 30),
};

struct SimEntity
{
	u32 StorageIndex;

	EntityType Type;
	u32 Flags;

	V2 Position;
	V2 Velocity;

	// This is for vertical change, aka stairs
	i32 TileZ;

	r32 Z;
	r32 dZ;

	i32 PositionZ;

	r32 Width, Height;

	u32 FacingDirection;

	r32 Speed;

	u32 MaxHp;
	Hitpoint Hitpoints[16];

	EntityReference SwordLowIndex;
	r32 DistanceRemaining;
};

struct SimEntityHash
{
	u32 Index;
	SimEntity *Ptr;
};

struct SimRegion
{
	World *World;
	WorldPosition Origin;
	R2 Bounds;

	u32 MaxEntitiesCount;
	u32 EntitiesCount;
	SimEntity *Entities;

	// Has to be power of 2!
	SimEntityHash Hash[4096];
};
