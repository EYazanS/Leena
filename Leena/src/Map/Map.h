#pragma once

#include "GameTypes.h"
#include "Utilities/Intrinsics.h"
#include "GameStructs.h"
#include "Memory/Memory.h"
#include "Math/Math.h"

struct MapPositionDifference
{
	V2 DXY;
	r32 DZ;
};

struct MapPosition
{
	// These are fixed points tile locations, the high bits are for tile chunk index, low bita are for tile index in the chunk
	i32 X;
	i32 Y;
	i32 Z;

	// Relative to the tile 
	V2 Offset;
};

struct WorldEntity
{
	u32 LowEntityIndex;
	MapPosition Position;
};

struct PositionChunks
{
	u32 EntitiesCount;
	WorldEntity Entities[30];
	PositionChunks* Next;
};

struct Map
{
	// We should see how many we can fit into world as one
	PositionChunks FirstChunkOfEntities;

	// How many tiles we have in each direction
	// it's unsinged so it's always positive
	// Max tiles in any direction is 65535
	// Max memory usage is 4 bytes * 65535 tiles * 3 direction = 786,420 bytes aka 768kb
	u16 TileCountX;
	u16 TileCountY;
	u16 TileCountZ;

	r32 TileSideInMeters;
};


// TODO: Add function to load map from file and set its value
void initializeMap(MemoryPool* pool, Map* map);
MapPosition MapIntoTileSpace(Map* map, MapPosition basePosition, V2 offset);
void RecanonicalizeCoordinant(Map* map, i32* tile, r32* tileRelative);
b32 AreOnSameTile(MapPosition position1, MapPosition position2);

inline MapPositionDifference CalculatePositionDifference(Map* map, MapPosition* position1, MapPosition* position2)
{
	MapPositionDifference result = {};

	V2 dTile = { (r32)position1->X - (r32)position2->X, (r32)position1->Y - (r32)position2->Y };
	r32 dTileZ = (r32)position1->Z - (r32)position2->Z;

	result.DXY = map->TileSideInMeters * dTile + (position1->Offset - position2->Offset);

	result.DZ = map->TileSideInMeters * dTileZ;

	return result;
}

inline MapPosition GenerateCeneteredTiledPosition(i32 x, i32 y, i32 z)
{
	MapPosition result = {};

	result = MapPosition{ x, y, z };

	return result;
}

inline b32 AreOnSameLocation(MapPosition* oldPosition, MapPosition* newPosition)
{
	b32 result = 0;

	return result;
}

inline void ChangeEntityLocation(MemoryPool* pool, Map* map, u32 index, MapPosition* oldPosition, MapPosition* newPosition)
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