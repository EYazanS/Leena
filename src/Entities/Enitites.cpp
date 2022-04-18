inline void MakeEntityNonSpatial(SimEntity *entity)
{
	AddFlag(entity, EntityFlag::Nonspatial);
	entity->Position = InvalidPosition;
}

inline void MakeEntitySpatial(SimEntity *entity, V2 position, V2 velocity)
{
	RemoveFlag(entity, EntityFlag::Nonspatial);
	entity->Position = position;
	entity->Velocity = velocity;
}