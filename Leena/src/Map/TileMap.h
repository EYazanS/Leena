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
	Vector2d DXY;
	real32 DZ;
};

struct MapPosition
{
	// These are fixed points tile locations, the high bits are for tile chunk index, low bita are for tile index in the chunk
	uint32 X;
	uint32 Y;
	uint32 Z;

	// Relative to the tile 
	Vector2d Offset;
};

struct Map
{
	TileValue* Tiles;

	// How many tiles we have in each direction
	// it's unsinged so it's always positive
	// Max tiles in any direction is 65535
	// Max memory usage is 4 bytes * 65535 tiles * 3 direction = 786,420 bytes aka 768kb
	uint16 TileCountX;
	uint16 TileCountY;
	uint16 TileCountZ;

	real32 TileSideInMeters;
};


// TODO: Add function to load map from file and set its value
void initializeMap(MemoryPool* pool, Map* map);
bool32 IsMapPointEmpty(Map* map, MapPosition position);
TileValue GetTileValue(Map* map, uint32 x, uint32 y, uint32 z);
TileValue GetTileValue(Map* map, MapPosition position);
void SetTileValue(Map* map, uint32 tileX, uint32 tileY, uint32 tileZ, TileValue value);
MapPosition RecanonicalizePosition(Map* map, MapPosition position);
void RecanonicalizeCoordinant(Map* map, uint32* tile, real32* tileRelative);
bool32 AreOnSameTile(MapPosition position1, MapPosition position2);

inline MapPositionDifference CalculatePositionDifference(Map* map, MapPosition* position1, MapPosition* position2)
{
	MapPositionDifference result = {};

	Vector2d dTile = { (real32)position1->X - (real32)position2->X, (real32)position1->Y - (real32)position2->Y };
	real32 dTileZ = (real32)position1->Z - (real32)position2->Z;

	result.DXY = map->TileSideInMeters * dTile + (position1->Offset - position2->Offset);

	result.DZ = map->TileSideInMeters * dTileZ;

	return result;
}

inline bool32 IsTileValueEmpty(TileValue tileValue)
{
	bool32 result = {};

	result = tileValue == TileValue::Empty || tileValue == TileValue::DoorUp || tileValue == TileValue::DoorDown;

	return result;
}

inline MapPosition GenerateCeneteredTiledPosition(uint32 x, uint32 y, uint32 z)
{
	MapPosition result = {};

	result = MapPosition{ x, y, z };

	return result;
}