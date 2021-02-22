#include "GameTypes.h"

struct TileChunkPosition
{
	int32 TileChunkX;
	int32 TileChunkY;

	int32 RelativeTileX;
	int32 RelativeTileY;
};

struct TileChunk
{
	uint32* Tiles;
};

struct Map
{
	uint32 TileChunkCountX;
	uint32 TileChunkCountY;

	uint32 ChunkShift;
	uint32 ChunkMask;
	uint32 ChunkDimension;

	real32 TileSideInMeters;
	int32 TileSideInPixels;
	real32 MetersToPixels;

	TileChunk* TileChunks;
};