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
	b32 result = (position.X != TileChunkUninitialized);

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

	b32 result = ((oldPosition->X == newPosition->X) &&
				  (oldPosition->Y == newPosition->Y) &&
				  (oldPosition->Z == newPosition->Z));

	return result;
}

inline WorldChunk *GetWorldChunk(
	World *world,
	i32 ChunkX,
	i32 ChunkY,
	i32 ChunkZ,
	MemoryPool *pool = 0)
{
	Assert(ChunkX > -ChunkSafeMargin);
	Assert(ChunkY > -ChunkSafeMargin);
	Assert(ChunkZ > -ChunkSafeMargin);
	Assert(ChunkX < ChunkSafeMargin);
	Assert(ChunkY < ChunkSafeMargin);
	Assert(ChunkZ < ChunkSafeMargin);

	// TODO(casey): BETTER HASH FUNCTION!!!!
	u32 HashValue = 19 * ChunkX + 7 * ChunkY + 3 * ChunkZ;
	u32 HashSlot = HashValue & (ArrayCount(world->TileChunkHash) - 1);
	Assert(HashSlot < ArrayCount(world->TileChunkHash));

	WorldChunk *Chunk = world->TileChunkHash + HashSlot;
	do
	{
		if ((ChunkX == Chunk->X) &&
			(ChunkY == Chunk->Y) &&
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
			Chunk->X = ChunkX;
			Chunk->Y = ChunkY;
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

inline void RecanonicalizeCoordinant(World *world, i32 *coordinate, r32 *coordRelative)
{
	// TODO(casey): Need to do something that doesn't use the divide/multiply method
	// for recanonicalizing because this can end up rounding back on to the tile
	// you just came from.

	// NOTE(casey): Wrapping IS NOT ALLOWED, so all coordinates are assumed to be
	// within the safe margin!
	// TODO(casey): Assert that we are nowhere near the edges of the world.

	i32 offset = RoundReal32ToInt32(*coordRelative / world->ChunkSideInMeters);
	*coordinate += offset;
	*coordRelative -= offset * world->ChunkSideInMeters;

	Assert(IsCanonical(world, *coordRelative));
}

inline WorldPosition MapIntoChunkSpace(World *world, WorldPosition basePosition, V2 offset)
{
	WorldPosition result = basePosition;
	result.Offset += offset;

	RecanonicalizeCoordinant(world, &result.X, &result.Offset.X);
	RecanonicalizeCoordinant(world, &result.Y, &result.Offset.Y);

	return result;
}

inline WorldPosition CenteredChunkPoint(u32 x, u32 y, u32 z)
{
	WorldPosition result = {};

	result.X = x;
	result.Y = y;
	result.Z = z;

	return (result);
}

inline void ChangeEntityLocationRaw(
	MemoryPool *Arena, World *World, u32 LowEntitiyIndex,
	WorldPosition *OldP, WorldPosition *NewP)
{
	// TODO(casey): If this moves an entity into the camera bounds, should it automatically
	// go into the high set immediately?
	// If it moves _out_ of the camera bounds, should it be removed from the high set
	// immediately?

	Assert(!OldP || IsValidPosition(*OldP));
	Assert(!NewP || IsValidPosition(*NewP));

	if (OldP && NewP && AreOnSameLocation(World, OldP, NewP))
	{
		// NOTE(casey): Leave entity where it is
	}
	else
	{
		if (OldP)
		{
			// NOTE(casey): Pull the entity out of its old entity block
			WorldChunk *Chunk = GetWorldChunk(World, OldP->X, OldP->Y, OldP->Z);

			Assert(Chunk);

			if (Chunk)
			{
				b32 NotFound = true;
				EntityBlock *FirstBlock = &Chunk->FirstBlock;
				for (EntityBlock *Block = FirstBlock;
					 Block && NotFound;
					 Block = Block->Next)
				{
					for (u32 Index = 0;
						 (Index < Block->EntitiesCount) && NotFound;
						 ++Index)
					{
						if (Block->LowEntitiyIndex[Index] == LowEntitiyIndex)
						{
							Assert(FirstBlock->EntitiesCount > 0);
							Block->LowEntitiyIndex[Index] =
								FirstBlock->LowEntitiyIndex[--FirstBlock->EntitiesCount];
							if (FirstBlock->EntitiesCount == 0)
							{
								if (FirstBlock->Next)
								{
									EntityBlock *NextBlock = FirstBlock->Next;
									*FirstBlock = *NextBlock;

									NextBlock->Next = World->FirstFreeBlock;
									World->FirstFreeBlock = NextBlock;
								}
							}

							NotFound = false;
						}
					}
				}
			}
		}

		if (NewP)
		{
			// NOTE(casey): Insert the entity into its new entity block
			WorldChunk *Chunk = GetWorldChunk(World, NewP->X, NewP->Y, NewP->Z, Arena);
			Assert(Chunk);

			EntityBlock *Block = &Chunk->FirstBlock;

			if (Block->EntitiesCount == ArrayCount(Block->LowEntitiyIndex))
			{
				// NOTE(casey): We're out of room, get a new block!
				EntityBlock *OldBlock = World->FirstFreeBlock;
				if (OldBlock)
				{
					World->FirstFreeBlock = OldBlock->Next;
				}
				else
				{
					OldBlock = PushStruct(Arena, EntityBlock);
				}

				*OldBlock = *Block;
				Block->Next = OldBlock;
				Block->EntitiesCount = 0;
			}

			Assert(Block->EntitiesCount < ArrayCount(Block->LowEntitiyIndex));
			Block->LowEntitiyIndex[Block->EntitiesCount++] = LowEntitiyIndex;
		}
	}
}

void ChangeEntityLocation(
	MemoryPool *Arena, World *World,
	u32 LowEntityIndex, LowEntity *LowEntity,
	WorldPosition NewPInit)
{
	WorldPosition *OldP = 0;
	WorldPosition *NewP = 0;

	if (!HasFlag(&LowEntity->Entity, EntityFlag::Nonspatial) && IsValidPosition(LowEntity->Position))
	{
		OldP = &LowEntity->Position;
	}

	if (IsValidPosition(NewPInit))
	{
		NewP = &NewPInit;
	}

	ChangeEntityLocationRaw(Arena, World, LowEntityIndex, OldP, NewP);

	if (NewP)
	{
		LowEntity->Position = *NewP;
		RemoveFlag(&LowEntity->Entity, EntityFlag::Nonspatial);
	}
	else
	{
		LowEntity->Position = NullPosition();
		AddFlag(&LowEntity->Entity, EntityFlag::Nonspatial);
	}
}
