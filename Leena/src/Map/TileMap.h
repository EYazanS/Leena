#pragma once

#include "GameTypes.h"
#include "Utilities/Intrinsics.h"
#include "GameStructs.h"
#include "Memory/Memory.h"
#include "Math/Math.h"

enum class TileValue
{
	Invalid = 0,
	Empty = 1,
	Wall = 2,
	Water = 3,
	DoorUp = 4,
	DoorDown = 5
};

struct MapPositionDifference
{
	V2 DXY;
	r32 DZ;
};

struct MapPosition
{
	// These are fixed points tile locations, the high bits are for tile chunk index, low bita are for tile index in the chunk
	u32 X;
	u32 Y;
	u32 Z;

	// Relative to the tile 
	V2 Offset;
};

struct Map
{
	TileValue* Tiles;

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
b32 IsMapPointEmpty(Map* map, MapPosition position);
TileValue GetTileValue(Map* map, u32 x, u32 y, u32 z);
TileValue GetTileValue(Map* map, MapPosition position);
void SetTileValue(Map* map, u32 tileX, u32 tileY, u32 tileZ, TileValue value);
MapPosition MapIntoTileSpace(Map* map, MapPosition basePosition, V2 offset);
void RecanonicalizeCoordinant(Map* map, u32* tile, r32* tileRelative);
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

inline b32 IsTileValueEmpty(TileValue tileValue)
{
	b32 result = {};

	result = tileValue == TileValue::Empty || tileValue == TileValue::DoorUp || tileValue == TileValue::DoorDown;

	return result;
}

inline MapPosition GenerateCeneteredTiledPosition(u32 x, u32 y, u32 z)
{
	MapPosition result = {};

	result = MapPosition{ x, y, z };

	return result;
}