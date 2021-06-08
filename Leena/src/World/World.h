#pragma once

#include "GameTypes.h"
#include "Utilities/Intrinsics.h"
#include "GameStructs.h"
#include "Memory/Memory.h"
#include "Math/Math.h"

#define ChunkSafeMargin 16
#define TilesPerChunk 16

struct WorldPositionDifference
{
	V2 DXY;
	r32 DZ;
};

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

// TODO: Add function to load map from file and set its value
void initializeWorld(World* world);
WorldPosition MapIntoChunkSpace(World* world, WorldPosition basePosition, V2 offset);
void RecanonicalizeCoordinant(World* world, i32* tile, r32* tileRelative);


inline b32 IsCanonical(World* world, r32 tileRel)
{
	b32 result = (tileRel >= (-0.5 * world->ChunkSideInMeters)) && (tileRel <= (0.5 * world->ChunkSideInMeters));

	return result;
}

inline b32 IsCanonical(World* world, V2 offset)
{
	b32 result = IsCanonical(world, offset.X) && IsCanonical(world, offset.Y);

	return result;
}

inline WorldPositionDifference CalculatePositionDifference(World* world, WorldPosition* position1, WorldPosition* position2)
{
	WorldPositionDifference result = {};

	V2 dTile = { (r32)position1->X - (r32)position2->X, (r32)position1->Y - (r32)position2->Y };
	r32 dTileZ = (r32)position1->Z - (r32)position2->Z;

	result.DXY = world->ChunkSideInMeters * dTile + (position1->Offset - position2->Offset);

	result.DZ = world->ChunkSideInMeters * dTileZ;

	return result;
}

inline WorldPosition GenerateChunkTiledPosition(i32 x, i32 y, i32 z)
{
	WorldPosition result = {};

	result = WorldPosition{ x, y, z };

	return result;
}

inline WorldPosition ChunkPositionFromWorldPosition(World* world, i32 x, i32 y, i32 z)
{
	WorldPosition result = {};

	// TOOD: handle Z
	result.X = (x / TilesPerChunk);
	result.Y = (y / TilesPerChunk);
	result.Z = (z / TilesPerChunk);

	result.Offset = V2{ (r32)(x - (result.X * TilesPerChunk) * world->TileSideInMeters), (r32)(y - (result.Y * TilesPerChunk) * world->TileSideInMeters) };

	return result;
}

inline b32 AreOnSameLocation(World* world, WorldPosition* oldPosition, WorldPosition* newPosition)
{
	Assert(IsCanonical(world, oldPosition->Offset));
	Assert(IsCanonical(world, newPosition->Offset));

	b32 result = oldPosition->X == newPosition->X && oldPosition->Y == newPosition->Y && oldPosition->Z == newPosition->Z;

	return result;
}

inline WorldChunk* GetWorldChunk(World* world, u32 x, u32 y, u32 z, MemoryPool* pool = 0)
{
	//Assert(x > ChunkSafeMargin);
	//Assert(z > ChunkSafeMargin);
	//Assert(y > ChunkSafeMargin);

	//Assert(x < (UINT32_MAX - ChunkSafeMargin));
	//Assert(z < (UINT32_MAX - ChunkSafeMargin));
	//Assert(y < (UINT32_MAX - ChunkSafeMargin));

	// TODO: Better hash function!
	u32 hashValue = 16 * x + 7 * z + 3 * y;
	u32 hashSlot = hashValue & (ArrayCount(world->TileChunkHash) - 1);
	Assert(hashSlot < ArrayCount(world->TileChunkHash));

	WorldChunk* chunk = world->TileChunkHash + hashSlot;

	do
	{
		if (chunk->X == x && chunk->Y == y && chunk->Z == z)
		{
			break;
		}

		if (pool && chunk->X != 0 && !chunk->NextInHash)
		{
			chunk->NextInHash = PushStruct(pool, WorldChunk);
			chunk->X = 0;
			chunk = chunk->NextInHash;
		}

		if (pool && chunk->X == 0)
		{
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

inline void ChangeEntityLocation(MemoryPool* pool, World* world, u32 lowEntityIndex, WorldPosition* oldPosition, WorldPosition* newPosition)
{
	if (oldPosition && AreOnSameLocation(world, oldPosition, newPosition))
	{
		// WE dont do anythink
	}
	else
	{
		if (oldPosition)
		{
			// Pull the entity from its current entity block
			WorldChunk* chunk = GetWorldChunk(world, oldPosition->X, oldPosition->Y, oldPosition->Z, pool);

			Assert(chunk);

			if (chunk)
			{
				EntityBlock* firstBlock = &chunk->FirstBlock;

				EntityBlock* block = &chunk->FirstBlock;

				while (block)
				{
					for (u32 index = 0; index < block->EntitiesCount; index++)
					{
						if (block->LowEntitiyIndex[index] == lowEntityIndex)
						{
							firstBlock->LowEntitiyIndex[index] = firstBlock->LowEntitiyIndex[--firstBlock->EntitiesCount];

							if (firstBlock->EntitiesCount == 0)
							{
								if (firstBlock->Next)
								{
									EntityBlock* nextBlock = firstBlock->Next;
									*firstBlock = *nextBlock;
									nextBlock->Next = world->FirstFreeBlock;
									world->FirstFreeBlock = nextBlock;
								}
							}

							block = 0;
							break;
						}
					}

					if (block)
						block = block->Next;
				}
			}
		}

		// Insert entity into its new entity block
		WorldChunk* chunk = GetWorldChunk(world, newPosition->X, newPosition->Y, newPosition->Z, pool);

		EntityBlock* block = &chunk->FirstBlock;

		if (block->EntitiesCount == ArrayCount(block->LowEntitiyIndex))
		{
			EntityBlock* oldBlock = world->FirstFreeBlock;

			if (oldBlock)
			{
				world->FirstFreeBlock = oldBlock->Next;
			}
			else
			{
				oldBlock = PushStruct(pool, EntityBlock);
			}

			*oldBlock = *block;
			block->Next = oldBlock;
			block->EntitiesCount = 0;
		}

		Assert(block->EntitiesCount < ArrayCount(block->LowEntitiyIndex));

		block->LowEntitiyIndex[block->EntitiesCount++] = lowEntityIndex;
	}
}