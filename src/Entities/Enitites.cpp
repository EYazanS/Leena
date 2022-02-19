void UpdateFamiliar(SimRegion *simRegion, SimEntity *entity, r32 timeDelta)
{
	r32 maximumSearchRadius = Square(5.0f);

	SimEntity *testEntity = simRegion->Entities;

	V2 ddP = {};

	for (u32 textEntityIndex = 0; textEntityIndex < simRegion->EntitiesCount; textEntityIndex++, ++testEntity)
	{
		if (testEntity->Type != EntityType::Player)
		{
			continue;
		}

		r32 testSq = LengthSq(testEntity->Position - entity->Position);

		if (testSq < maximumSearchRadius && testSq > 0.01f)
		{
			r32 acceleration = 0.5f;

			r32 oneOverLength = acceleration / SquareRoot(testSq);

			ddP = oneOverLength * (testEntity->Position - entity->Position);
		}

		break;
	}

	MoveSpec moveSpec = GetDefaultMoveSpec();

	moveSpec.Drag = 8.0f;
	moveSpec.Speed = 50.0f;
	moveSpec.UnitMaxAccVector = true;

	MoveEntity(simRegion, entity, timeDelta, ddP, &moveSpec);
}

void UpdateMonster(SimRegion *simRegion, SimEntity *entity, r32 dt)
{
}

void UpdateSword(SimRegion *simRegion, SimEntity *entity, r32 dt)
{
	MoveSpec moveSpec = GetDefaultMoveSpec();

	moveSpec.Speed = 0.0f;

	V2 oldPosition = entity->Position;

	MoveEntity(simRegion, entity, dt, {0, 0}, &moveSpec);

	r32 distanceTraveled = Length(entity->Position - oldPosition);

	entity->DistanceRemaining -= distanceTraveled;

	if (entity->DistanceRemaining < 0.0f)
	{
		Assert(!"Make entity to move out of region")

		// ChangeEntityLocation(
		// 	&state->WorldMemoryPool,
		// 	state->World,
		// 	entity->StorageIndex,
		// 	entity,
		// 	&entity.Position, 0);
	}
}