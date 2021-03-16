#pragma once

#include "GameTypes.h"
#include "Utilities/Intrinsics.h"
#include "GameStructs.h"
#include "Memory/Memory.h"

enum class TileValue
{
	Invalid = 0,
	Empty = 1,
	Wall = 2,
	Water = 3,
	DoorUp = 4,
	DoorDown = 5
};


struct MapPosition
{
	// These are fixed points tile locations, the high bits are for tile chunk index, low bita are for tile index in the chunk
	uint32 X;
	uint32 Y;
	uint32 Z;

	// Relative to the tile 
	real32 TileRelativeX;
	real32 TileRelativeY;
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
void initializeMap(MemoryPool* pool, Map* map);
bool32 IsMapPointEmpty(Map* map, MapPosition position);
TileValue GetTileValue(Map* map, uint32 absTileX, uint32 absTileY, uint32 absTileZ);
void SetTileValue(Map* map, uint32 tileX, uint32 tileY, uint32 tileZ, TileValue value);
MapPosition RecanonicalizePosition(Map* map, MapPosition position);
void RecanonicalizeCoordinant(Map* map, uint32* tile, real32* tileRelative);