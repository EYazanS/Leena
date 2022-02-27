#pragma once

#define InvalidPosition {100000.0f, 100000.0f}

struct MoveSpec
{
	b32 UnitMaxAccVector;
	r32 Drag;
	r32 Speed;
};

inline b32 HasFlag(SimEntity *entity, u32 flags)
{
	b32 result = entity->Flags & flags;
	return result;
}

inline void AddFlag(SimEntity *entity, u32 flags)
{
	entity->Flags |= flags;
}

inline void RemoveFlag(SimEntity *entity, u32 flags)
{
	entity->Flags &= ~flags;
}