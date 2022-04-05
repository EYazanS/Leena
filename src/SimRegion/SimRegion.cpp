SimEntity *AddEntity(GameState *gameState, SimRegion *simRegion, u32 storedIndex, LowEntity *source, V2 *position);

inline V2 GetSimSpacePosition(SimRegion *simRegion, LowEntity *entity)
{
	V2 resutlt = InvalidPosition;

	if (!HasFlag(&entity->Entity, EntityFlag::Nonspatial))
	{
		V3 diff = CalculatePositionDifference(simRegion->World, &entity->Position, &simRegion->Origin);
		resutlt = V2{diff.X, diff.Y};
	}

	return resutlt;
}

SimEntityHash *GetHashFromStorageIndex(SimRegion *simRegion, u32 storageIndex)
{
	Assert(storageIndex);

	SimEntityHash *result = 0;

	u32 hashValue = storageIndex;

	for (u32 offset = 0; offset < ArrayCount(simRegion->Hash); offset++)
	{
		u32 hashIndex = (hashValue + offset) & (ArrayCount(simRegion->Hash) - 1);

		SimEntityHash *entry = simRegion->Hash + hashIndex;

		if (entry->Index == storageIndex || entry->Index == 0)
		{
			result = entry;
			break;
		}
	}

	return result;
}

void LoadEntityReference(GameState *gameState, SimRegion *simRegion, EntityReference *reference)
{
	if (reference->Index)
	{
		SimEntityHash *entry = GetHashFromStorageIndex(simRegion, reference->Index);

		if (!entry->Ptr)
		{
			entry->Index = reference->Index;
			entry->Ptr = AddEntity(gameState, simRegion, reference->Index, GetLowEntity(gameState, reference->Index), 0);
		}

		reference->Ptr = entry->Ptr;
	}
}

void StoreEntityReference(EntityReference *reference)
{
	if (reference->Ptr != 0)
	{
		reference->Index = reference->Ptr->StorageIndex;
	}
}

SimEntity *AddEntityRaw(GameState *gameState, SimRegion *simRegion, u32 storageIndex, LowEntity *source)
{
	Assert(storageIndex);

	SimEntity *result = 0;

	SimEntityHash *entry = GetHashFromStorageIndex(simRegion, storageIndex);

	if (entry->Ptr == 0)
	{
		if (simRegion->EntitiesCount < simRegion->MaxEntitiesCount)
		{
			result = simRegion->Entities + simRegion->EntitiesCount++;

			entry->Index = storageIndex;
			entry->Ptr = result;

			if (source)
			{
				*result = source->Entity;
				LoadEntityReference(gameState, simRegion, &result->SwordLowIndex);
				Assert(!HasFlag(&source->Entity, EntityFlag::Simming));
				AddFlag(&source->Entity, EntityFlag::Simming);
			}

			result->StorageIndex = storageIndex;
		}
		else
		{
			InvalidCodePath;
		}
	}

	return result;
}

SimEntity *AddEntity(GameState *gameState, SimRegion *simRegion, u32 storedIndex, LowEntity *source, V2 *position)
{
	SimEntity *dest = AddEntityRaw(gameState, simRegion, storedIndex, source);

	if (dest)
	{
		// Convert stored entity to sim entity
		if (position)
		{
			dest->Position = *position;
		}
		else
		{
			dest->Position = GetSimSpacePosition(simRegion, source);
		}
	}

	return dest;
}

SimRegion *BeginSim(MemoryPool *pool, GameState *gameState, World *world, R2 bounds, WorldPosition origin)
{
	SimRegion *simRegion = PushStruct(pool, SimRegion);

	ZeroStruct(simRegion->Hash);

	simRegion->World = world;
	simRegion->Origin = origin;
	simRegion->Bounds = bounds;

	simRegion->MaxEntitiesCount = 4096;
	simRegion->EntitiesCount = 0;
	simRegion->Entities = PushArray(pool, simRegion->MaxEntitiesCount, SimEntity);

	WorldPosition minChunkPos = MapIntoChunkSpace(world, simRegion->Origin, GetMinCorner(simRegion->Bounds));
	WorldPosition maxChunkPos = MapIntoChunkSpace(world, simRegion->Origin, GetMaxCorner(simRegion->Bounds));

	for (i32 chunkY = minChunkPos.Y; chunkY <= maxChunkPos.Y; ++chunkY)
	{
		for (i32 chunkX = minChunkPos.X; chunkX <= maxChunkPos.X; ++chunkX)
		{
			WorldChunk *chunk = GetWorldChunk(world, chunkX, chunkY, simRegion->Origin.Z);

			if (chunk)
			{
				for (EntityBlock *block = &chunk->FirstBlock; block; block = block->Next)
				{
					for (u32 entityIndex = 0; entityIndex < block->EntitiesCount; ++entityIndex)
					{
						u32 lowEntityIndex = block->LowEntitiyIndex[entityIndex];

						LowEntity *storedEntity = gameState->LowEntities + lowEntityIndex;

						if (!HasFlag(&storedEntity->Entity, EntityFlag::Nonspatial))
						{
							V2 simSpaceP = GetSimSpacePosition(simRegion, storedEntity);

							if (IsInRectangle(simRegion->Bounds, simSpaceP))
							{
								AddEntity(gameState, simRegion, lowEntityIndex, storedEntity, &simSpaceP);
							}
						}
					}
				}
			}
		}
	}

	return simRegion;
}

b32 TestWall(r32 wall, r32 relX, r32 relY, r32 playerDeltaX, r32 playerDeltaY, r32 *tMin, r32 minY, r32 maxY)
{
	b32 hitWall = false;
	r32 tEpslion = 0.0001f;

	if (playerDeltaX != 0)
	{
		r32 tResult = (wall - relX) / playerDeltaX;

		r32 y = relY + tResult * playerDeltaY;

		if ((tResult >= 0.0f) && (*tMin > tResult))
		{
			if ((y >= minY) && (y <= maxY))
			{
				*tMin = Maximum(0.0f, tResult - tEpslion);
				hitWall = true;
			}
		}
	}

	return hitWall;
}

void MoveEntity(
	SimRegion *simRegion,
	SimEntity *entity,
	r32 timeToAdvance,
	V2 playerAcceleration,
	MoveSpec *moveSpec)
{
	Assert(!HasFlag(entity, EntityFlag::Nonspatial));

	World *world = simRegion->World;

	if (moveSpec->UnitMaxAccVector)
	{
		r32 playerAccelerationLength = LengthSq(playerAcceleration);

		if (playerAccelerationLength > 1.0f)
		{
			playerAcceleration *= (1.0f / SquareRoot(playerAccelerationLength));
		}
	}

	playerAcceleration *= moveSpec->Speed;

	// TODO: ODE here!
	playerAcceleration += -moveSpec->Drag * entity->Velocity;

	V2 oldPlayerPosition = entity->Position;
	V2 playerDelta = (0.5f * playerAcceleration * Square(timeToAdvance) + entity->Velocity * timeToAdvance);

	entity->Velocity = playerAcceleration * timeToAdvance + entity->Velocity;

	V2 newPlayerPosition = oldPlayerPosition + playerDelta;

	for (u32 iteration = 0; iteration < 4; iteration++)
	{
		r32 tMin = 1.0f;
		V2 wallNormal = {};
		SimEntity *hitEntity = 0;

		V2 desiredPosition = entity->Position + playerDelta;

		if (HasFlag(entity, EntityFlag::Collides) && !HasFlag(entity, EntityFlag::Nonspatial))
		{
			for (u32 testEntityIndex = 1; testEntityIndex < simRegion->EntitiesCount; testEntityIndex++)
			{
				SimEntity *testEntity = 0;

				testEntity = simRegion->Entities + testEntityIndex;

				if (testEntity != entity &&
					HasFlag(testEntity, EntityFlag::Collides) &&
					!HasFlag(testEntity, EntityFlag::Nonspatial))
				{
					r32 diameterWidth = testEntity->Width + entity->Width;
					r32 diameterHeight = testEntity->Height + entity->Height;

					V2 minCorner = -0.5f * V2{diameterWidth, diameterHeight};
					V2 maxCorner = 0.5f * V2{diameterWidth, diameterHeight};

					V2 relDifference = entity->Position - testEntity->Position;

					if (TestWall(minCorner.X, relDifference.X, relDifference.Y, playerDelta.X, playerDelta.Y, &tMin, minCorner.Y, maxCorner.Y))
					{
						wallNormal = V2{-1, 0};
						hitEntity = testEntity;
					}

					if (TestWall(maxCorner.X, relDifference.X, relDifference.Y, playerDelta.X, playerDelta.Y, &tMin, minCorner.Y, maxCorner.Y))
					{
						wallNormal = V2{1, 0};
						hitEntity = testEntity;
					}

					if (TestWall(minCorner.Y, relDifference.Y, relDifference.X, playerDelta.Y, playerDelta.X, &tMin, minCorner.X, maxCorner.X))
					{
						wallNormal = V2{0, -1};
						hitEntity = testEntity;
					}

					if (TestWall(maxCorner.Y, relDifference.Y, relDifference.X, playerDelta.Y, playerDelta.X, &tMin, minCorner.X, maxCorner.X))
					{
						wallNormal = V2{0, 1};
						hitEntity = testEntity;
					}
				}
			}
		}

		entity->Position += tMin * playerDelta;

		if (hitEntity)
		{
			entity->Velocity = entity->Velocity - 1 * InnerProduct(entity->Velocity, wallNormal) * wallNormal;
			playerDelta = desiredPosition - entity->Position;
			playerDelta = playerDelta - 1 * InnerProduct(playerDelta, wallNormal) * wallNormal;

			// TODO: stairs
			// entity->AbsTileZ += HitdAbsTileZ;
		}
		else
		{
			break;
		}
	}

	// TODO: Change to using the acceleration vector
	if ((entity->Velocity.X == 0.0f) && (entity->Velocity.Y == 0.0f))
	{
		// NOTE: Leave FacingDirection whatever it was
	}
	else if (AbsoluteValue(entity->Velocity.X) > AbsoluteValue(entity->Velocity.Y))
	{
		if (entity->Velocity.X > 0)
		{
			entity->FacingDirection = 0;
		}
		else
		{
			entity->FacingDirection = 2;
		}
	}
	else
	{
		if (entity->Velocity.Y > 0)
		{
			entity->FacingDirection = 1;
		}
		else
		{
			entity->FacingDirection = 3;
		}
	}
}

void EndSim(SimRegion *region, GameState *gameState)
{
	SimEntity *entity = region->Entities;

	for (u32 entityIndex = 0; entityIndex < region->EntitiesCount; entityIndex++, ++entity)
	{
		LowEntity *stored = gameState->LowEntities + entity->StorageIndex;

		Assert(HasFlag(&stored->Entity, EntityFlag::Simming));
		stored->Entity = *entity;
		Assert(!HasFlag(&stored->Entity, EntityFlag::Simming));

		StoreEntityReference(&stored->Entity.SwordLowIndex);

		WorldPosition newPosition = HasFlag(entity, EntityFlag::Nonspatial)
										? NullPosition()
										: MapIntoChunkSpace(region->World, region->Origin, entity->Position);

		ChangeEntityLocation(
			&gameState->WorldMemoryPool,
			region->World,
			entity->StorageIndex,
			stored,
			newPosition);

		// Smooth scrolling for the camera
	}

	WorldPosition NewCameraP = (gameState->LowEntities + gameState->PlayerEntity->StorageIndex)->Position;
	gameState->CameraPosition = NewCameraP;
}