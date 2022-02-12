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

struct MoveSpec
{
	b32 UnitMaxAccVector;
	r32 Drag;
	r32 Speed;
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

struct SimEntity
{
	V2 Position;
	V2 Velocity;
	i32 PositionZ;

	u32 FacingDirection;

	r32 Z;
	r32 dZ;

	u32 StorageIndex;

	EntityType Type;
	r32 Width, Height;
	r32 Speed;
	b32 Collides;

	// This is for vertical change, aka stairs
	i32 TileZ;

	u32 MaxHp;
	Hitpoint Hitpoints[16];

	EntityReference SwordLowIndex;

	r32 Timer;

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
