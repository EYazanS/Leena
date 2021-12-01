#pragma once

#include "GameTypes.h"
#include "Utilities/Intrinsics.h"
#include "GameStructs.h"
#include "Memory/Memory.h"
#include "Math/Math.h"

#define ChunkSafeMargin 16
#define TilesPerChunk 16
#define TileChunkUninitialized INT32_MAX

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

inline WorldPosition NullPosition()
{
	WorldPosition result = {};

	result.X = TileChunkUninitialized;
	result.Y = TileChunkUninitialized;
	result.Z = TileChunkUninitialized;

	return result;
}

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

inline V3 CalculatePositionDifference(World* world, WorldPosition* position1, WorldPosition* position2)
{
	V3 result = {};

	V2 dTile = { (r32)position1->X - (r32)position2->X, (r32)position1->Y - (r32)position2->Y };
	r32 dTileZ = (r32)position1->Z - (r32)position2->Z;

	V2 v2 = world->ChunkSideInMeters * dTile + (position1->Offset - position2->Offset);

	result = { v2.X, v2.Y, world->ChunkSideInMeters * dTileZ };

	return result;
}

inline WorldPosition GetChunkPositionFromWorldPosition(World* world, i32 x, i32 y, i32 z)
{
	WorldPosition result = {};

	result.X = x / TilesPerChunk;
	result.Y = y / TilesPerChunk;
	result.Z = z / TilesPerChunk;

	// TODO: Think this through on the real stream and actually work out the math.
	if (x < 0)
	{
		--result.X;
	}
	if (y < 0)
	{
		--result.Y;
	}
	if (z < 0)
	{
		--result.Z;
	}

	// TODO: DECIDE ON TILE ALIGNMENT IN CHUNKS!
	result.Offset.X = (r32)((x - TilesPerChunk / 2) - (result.X * TilesPerChunk)) * world->TileSideInMeters;
	result.Offset.Y = (r32)((y - TilesPerChunk / 2) - (result.Y * TilesPerChunk)) * world->TileSideInMeters;

	Assert(IsCanonical(world, result.Offset));

	return result;
}

inline b32 AreOnSameLocation(World* world, WorldPosition* oldPosition, WorldPosition* newPosition)
{
	Assert(IsCanonical(world, oldPosition->Offset));
	Assert(IsCanonical(world, newPosition->Offset));

	b32 result = oldPosition->X == newPosition->X && oldPosition->Y == newPosition->Y && oldPosition->Z == newPosition->Z;

	return result;
}
inline b32 IsValidLocation(WorldPosition* position)
{

	b32 result = position->X != TileChunkUninitialized && position->Y != TileChunkUninitialized && position->Z != TileChunkUninitialized;

	return result;
}

inline WorldChunk* GetWorldChunk(World* world, u32 X, u32 Y, u32 ChunkZ, MemoryPool* pool = 0)
{
	//Assert(x > ChunkSafeMargin);
	//Assert(z > ChunkSafeMargin);
	//Assert(y > ChunkSafeMargin);

	//Assert(x < (u32_MAX - ChunkSafeMargin));
	//Assert(z < (u32_MAX - ChunkSafeMargin));
	//Assert(y < (u32_MAX - ChunkSafeMargin));

	// TODO: Better hash function!
	u32 HashValue = 19 * X + 7 * Y + 3 * ChunkZ;
	u32 HashSlot = HashValue & (ArrayCount(world->TileChunkHash) - 1);
	Assert(HashSlot < ArrayCount(world->TileChunkHash));

	WorldChunk* Chunk = world->TileChunkHash + HashSlot;
	do
	{
		if ((X == Chunk->X) &&
			(Y == Chunk->Y) &&
			(ChunkZ == Chunk->Z))
		{
			break;
		}

		if (pool && (Chunk->X != TileChunkUninitialized) && (!Chunk->NextInHash))
		{
			Chunk->NextInHash = PushStruct(pool, WorldChunk);
			Chunk = Chunk->NextInHash;
			Chunk->X = TileChunkUninitialized;
		}

		if (pool && (Chunk->X == TileChunkUninitialized))
		{
			Chunk->X = X;
			Chunk->Y = Y;
			Chunk->Z = ChunkZ;

			Chunk->NextInHash = 0;
			break;
		}

		Chunk = Chunk->NextInHash;
	} while (Chunk);

	return(Chunk);
}

inline void ChangeEntityLocationRaw(MemoryPool* pool, World* world, u32 lowEntityIndex, WorldPosition* oldPosition, WorldPosition* newPosition)
{
	Assert(!oldPosition || IsValidLocation(oldPosition));
	Assert(!newPosition || IsValidLocation(newPosition));

	if (oldPosition && newPosition && AreOnSameLocation(world, oldPosition, newPosition))
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

		if (newPosition)
		{
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

			if (block->EntitiesCount < ArrayCount(block->LowEntitiyIndex))
			{
				block->LowEntitiyIndex[block->EntitiesCount++] = lowEntityIndex;
			}
		}
	}
}