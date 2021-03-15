#pragma once

#include "GameTypes.h"
#include "Utilities/Intrinsics.h"
#include "GameStructs.h"
#include "Memory/Memory.h"

enum TileValue
{
	Empty = 1,
	Wall = 2,
	Water = 3
};

struct TileChunkPosition
{
	int32 TileChunkX;
	int32 TileChunkY;
	int32 TileChunkZ;

	int32 RelativeTileX;
	int32 RelativeTileY;
};

struct TileChunk
{
	uint32* Tiles;
};

struct Map
{
	uint32 ChunkShift;
	uint32 ChunkMask;
	uint32 ChunkDimension;

	real32 TileSideInMeters;

	uint32 TileChunkCountX;
	uint32 TileChunkCountY;
	uint32 TileChunkCountZ;

	TileChunk* TileChunks;
};

struct TileMapPosition
{
	// These are fixed points tile locations, the high bits are for tile chunk index, low bita are for tile index in the chunk
	uint32 AbsTileX;
	uint32 AbsTileY;
	uint32 AbsTileZ;

	// Tile relative
	real32 TileRelativeX;
	real32 TileRelativeY;
};


bool32 IsMapPointEmpty(Map* map, TileMapPosition position);
uint32 GetTileValue(Map* map, uint32 absTileX, uint32 absTileY, uint32 absTileZ);
uint32 GetTileValue(Map* map, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY);
TileMapPosition RecanonicalizePosition(Map* map, TileMapPosition position);
void RecanonicalizeCoordinant(Map* map, uint32* tile, real32* tileRelative);
TileChunkPosition GetTileChunkPosition(Map* map, uint32 absTileX, uint32 absTileY, uint32 absTileZ);
TileChunk* GetTileChunk(Map* map, uint32 tileChunkX, uint32 tileChunkY, uint32 tileChunkZ);
int32 GetTileValueUnchecked(Map* map, TileChunk* tileChunk, uint32 tileX, uint32 tileY);
void SetTileValue(MemoryPool* pool, Map* map, uint32 tileX, uint32 tileY, uint32 tileZ, uint32 value);