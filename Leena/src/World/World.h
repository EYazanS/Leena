#pragma once

#include "GameTypes.h"
#include "Utilities/Intrinsics.h"
#include "GameStructs.h"
#include "Memory/Memory.h"
#include "Math/Math.h"

#define ChunkSafeMargin 16

struct WorldPositionDifference
{
	V2 DXY;
	r32 DZ;
};

struct WorldPosition
{
	// These are fixed points tile locations, the high bits are for tile chunk index, low bita are for tile index in the chunk
	i32 X;
	i32 Y;
	i32 Z;

	// Relative to the tile 
	V2 Offset;
};

struct EntityBlock
{
	u32 EntityCount;
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

	i32 ChunkShift, ChunkMask, ChunkDim;

	// We should see how many we can fit into world as one
	WorldChunk TileChunkHash[4096];
};

// TODO: Add function to load map from file and set its value
void initializeWorld(World* map);
WorldPosition MapIntoWorldSpace(World* map, WorldPosition basePosition, V2 offset);
void RecanonicalizeCoordinant(World* map, i32* tile, r32* tileRelative);
b32 AreOnSameTile(WorldPosition position1, WorldPosition position2);

inline WorldPositionDifference CalculatePositionDifference(World* map, WorldPosition* position1, WorldPosition* position2)
{
	WorldPositionDifference result = {};

	V2 dTile = { (r32)position1->X - (r32)position2->X, (r32)position1->Y - (r32)position2->Y };
	r32 dTileZ = (r32)position1->Z - (r32)position2->Z;

	result.DXY = map->TileSideInMeters * dTile + (position1->Offset - position2->Offset);

	result.DZ = map->TileSideInMeters * dTileZ;

	return result;
}

inline WorldPosition GenerateCeneteredTiledPosition(i32 x, i32 y, i32 z)
{
	WorldPosition result = {};

	result = WorldPosition{ x, y, z };

	return result;
}

inline b32 AreOnSameLocation(WorldPosition* oldPosition, WorldPosition* newPosition)
{
	b32 result = 0;

	return result;
}

#if 0
inline void ChangeEntityLocation(MemoryPool* pool, World* map, u32 index, WorldPosition* oldPosition, WorldPosition* newPosition)
{
	if (!oldPosition || !AreOnSameLocation(oldPosition, newPosition))
	{
		if (oldPosition)
		{

		}

		// Insert entity into its new position

		PositionChunks chunk = map->FirstChunkOfEntities;

		u32 newIndex = chunk.EntitiesCount + 1;

		if (chunk.EntitiesCount == ArrayCount(chunk.Entities))
		{
			PositionChunks* oldChunk = PushSize(pool, PositionChunks);
			*oldChunk = chunk;
			chunk.Next = oldChunk;
			chunk.EntitiesCount = 0;
		}

		chunk.Entities[newIndex].LowEntityIndex = index;
		chunk.Entities[newIndex].Position = *newPosition;

		chunk.EntitiesCount = newIndex;
	}
}
#endif

inline WorldChunk* GetMaChunk(World* map, u32 x, u32 y, u32 z, MemoryPool* pool = 0)
{
	Assert(x > ChunkSafeMargin);
	Assert(z > ChunkSafeMargin);
	Assert(y > ChunkSafeMargin);

	Assert(x < (UINT32_MAX - ChunkSafeMargin));
	Assert(z < (UINT32_MAX - ChunkSafeMargin));
	Assert(y < (UINT32_MAX - ChunkSafeMargin));

	// TODO: Better hash function!
	u32 hashValue = 16 * x + 7 * z + 3 * y;
	u32 hashSlot = hashValue & (ArrayCount(map->TileChunkHash) - 1);
	Assert(hashSlot < ArrayCount(map->TileChunkHash));

	WorldChunk* chunk = map->TileChunkHash + hashSlot;

	do
	{
		if (chunk->X == x && chunk->Y == y && chunk->Z == z)
		{
			break;
		}

		if (pool && chunk->X != 0 && !chunk->NextInHash)
		{
			chunk->NextInHash = PushSize(pool, WorldChunk);
			chunk->X = 0;
			chunk = chunk->NextInHash;
		}

		if (pool && chunk->X == 0)
		{
			// Each chunk is Chunk dim squared
			u32 tilesCount = map->ChunkDim * map->ChunkDim;

			chunk->X = x;
			chunk->Y = y;
			chunk->Z = z;

			chunk->NextInHash = 0;

			break;
		}

		chunk = chunk->NextInHash;
	} while (chunk);

	return chunk;
}