#define TilesPerChunk 16
#define ChunkSafeMargin (INT32_MAX / 64)
#define TileChunkUninitialized INT32_MAX

inline WorldPosition NullPosition()
{
	WorldPosition result = {};

	result.X = TileChunkUninitialized;
	result.Y = TileChunkUninitialized;
	result.Z = TileChunkUninitialized;

	return result;
}

inline b32 IsValidPosition(WorldPosition position)
{

	b32 result = position.X != TileChunkUninitialized;

	return result;
}

inline b32 IsCanonical(World *world, r32 tileRel)
{
	r32 epsilon = 0.0001f;

	b32 result = ((tileRel >= -(0.5 * world->ChunkSideInMeters + epsilon)) && (tileRel <= (0.5 * world->ChunkSideInMeters + epsilon)));

	return result;
}

inline b32 IsCanonical(World *world, V2 offset)
{
	b32 result = IsCanonical(world, offset.X) && IsCanonical(world, offset.Y);

	return result;
}

inline b32 AreOnSameLocation(World *world, WorldPosition *oldPosition, WorldPosition *newPosition)
{
	Assert(IsCanonical(world, oldPosition->Offset));
	Assert(IsCanonical(world, newPosition->Offset));

	b32 result = oldPosition->X == newPosition->X && oldPosition->Y == newPosition->Y && oldPosition->Z == newPosition->Z;

	return result;
}

inline WorldChunk *GetWorldChunk(World *world, u32 X, u32 Y, u32 ChunkZ, MemoryPool *pool = 0)
{
	// Assert(x > ChunkSafeMargin);
	// Assert(z > ChunkSafeMargin);
	// Assert(y > ChunkSafeMargin);

	// Assert(x < (u32_MAX - ChunkSafeMargin));
	// Assert(z < (u32_MAX - ChunkSafeMargin));
	// Assert(y < (u32_MAX - ChunkSafeMargin));

	// TODO: Better hash function!
	u32 HashValue = 19 * X + 7 * Y + 3 * ChunkZ;
	u32 HashSlot = HashValue & (ArrayCount(world->TileChunkHash) - 1);
	Assert(HashSlot < ArrayCount(world->TileChunkHash));

	WorldChunk *Chunk = world->TileChunkHash + HashSlot;
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

	return (Chunk);
}

inline WorldPosition GetChunkPositionFromWorldPosition(World *world, i32 x, i32 y, i32 z)
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

inline V3 CalculatePositionDifference(World *world, WorldPosition *position1, WorldPosition *position2)
{
	V3 result = {};

	V2 dTile = {(r32)position1->X - (r32)position2->X, (r32)position1->Y - (r32)position2->Y};
	
	r32 dTileZ = (r32)position1->Z - (r32)position2->Z;

	V2 v2 = world->ChunkSideInMeters * dTile + (position1->Offset - position2->Offset);

	result = {v2.X, v2.Y, world->ChunkSideInMeters * dTileZ};

	return result;
}

void initializeWorld(World *world)
{
	if (world)
	{
		world->TileSideInMeters = 1.4f;

		world->ChunkSideInMeters = TilesPerChunk * world->TileSideInMeters;

		world->FirstFreeBlock = 0;

		for (u32 tileChunkIndex = 0; tileChunkIndex < ArrayCount(world->TileChunkHash); tileChunkIndex++)
		{
			world->TileChunkHash[tileChunkIndex].X = 0;
			world->TileChunkHash[tileChunkIndex].FirstBlock.EntitiesCount = 0;
		}
	}
}

void RecanonicalizeCoordinant(World *world, i32 *coordinate, r32 *coordRelative)
{
	// Offset from the current tile center, if its above 1 then it means we moved one tile
	i32 Offset = RoundReal32ToInt32(*coordRelative / world->ChunkSideInMeters);
	*coordinate += Offset;
	*coordRelative -= (Offset * world->ChunkSideInMeters);

	Assert(IsCanonical(world, *coordRelative));
}

WorldPosition MapIntoChunkSpace(World *world, WorldPosition basePosition, V2 offset)
{
	WorldPosition result = basePosition;
	result.Offset += offset;

	RecanonicalizeCoordinant(world, &result.X, &result.Offset.X);
	RecanonicalizeCoordinant(world, &result.Y, &result.Offset.Y);

	return result;
}

inline WorldPosition CenteredChunkPoint(u32 x, u32 y, u32 z)
{
	WorldPosition Result = {};

	Result.X = x;
	Result.Y = y;
	Result.Z = z;

	return (Result);
}

inline void ChangeEntityLocationRaw(
	MemoryPool *pool,
	World *world,
	u32 lowEntityIndex,
	WorldPosition *oldPosition,
	WorldPosition *newPosition)
{
	Assert(!oldPosition || IsValidPosition(*oldPosition));
	Assert(!newPosition || IsValidPosition(*newPosition));

	if (oldPosition && newPosition && AreOnSameLocation(world, oldPosition, newPosition))
	{
		// WE dont do anythink
	}
	else
	{
		if (oldPosition)
		{
			// Pull the entity from its current entity block
			WorldChunk *chunk = GetWorldChunk(world, oldPosition->X, oldPosition->Y, oldPosition->Z, pool);

			Assert(chunk);

			if (chunk)
			{
				EntityBlock *firstBlock = &chunk->FirstBlock;

				EntityBlock *block = &chunk->FirstBlock;

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
									EntityBlock *nextBlock = firstBlock->Next;
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
			WorldChunk *chunk = GetWorldChunk(world, newPosition->X, newPosition->Y, newPosition->Z, pool);

			EntityBlock *block = &chunk->FirstBlock;

			if (block->EntitiesCount == ArrayCount(block->LowEntitiyIndex))
			{
				EntityBlock *oldBlock = world->FirstFreeBlock;

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

void ChangeEntityLocation(
	MemoryPool *pool,
	World *world,
	u32 lowEntityIndex,
	LowEntity *lowEntity,
	WorldPosition newPInit)
{
	WorldPosition *oldPosition = 0;
	WorldPosition *newPosition = 0;

	if (!HasFlag(&lowEntity->Entity, EntityFlag::Nonspatial) && IsValidPosition(lowEntity->Position))
	{
		oldPosition = &lowEntity->Position;
	}

	if (IsValidPosition(newPInit))
	{
		newPosition = &newPInit;
	}

	ChangeEntityLocationRaw(pool, world, lowEntityIndex, oldPosition, newPosition);

	if (newPosition)
	{
		lowEntity->Position = *newPosition;
		RemoveFlag(&lowEntity->Entity, EntityFlag::Nonspatial);
	}
	else
	{
		lowEntity->Position = NullPosition();
		AddFlag(&lowEntity->Entity, EntityFlag::Nonspatial);
	}
}