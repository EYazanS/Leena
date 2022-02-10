#pragma once

#include "GameStructs.h"

struct WorldPosition
{
	i32 X;
	i32 Y;
	i32 Z;

	// Offset from the chunk center
	V2 Offset;
};

struct EntityBlock
{
	u32 EntitiesCount;
	u32 LowEntitiyIndex[16];
	EntityBlock* Next;
};

struct WorldChunk
{
	u32 X, Y, Z;

	EntityBlock FirstBlock;

	WorldChunk* NextInHash;
};

struct World
{
	r32 TileSideInMeters;
	r32 ChunkSideInMeters;

	// We should see how many we can fit into world as one
	WorldChunk TileChunkHash[4096];

	EntityBlock* FirstFreeBlock;
};