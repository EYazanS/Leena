#include "TileMap.h"

 TileChunk* GetTileChunk(Map* map, uint32 tileChunkX, uint32 tileChunkY)
{
	TileChunk* tileChunk = 0;

	if (tileChunkX <= map->TileChunkCountX && tileChunkY <= map->TileChunkCountY)
		tileChunk = &map->TileChunks[tileChunkY * map->TileChunkCountX + tileChunkX];

	return tileChunk;
}

 int32 GetTileValueUnchecked(TileChunk* tileChunk, uint32 tileCountX, uint32 tileX, uint32 tileY)
{
	return tileChunk->Tiles[tileY * tileCountX + tileX];
}

 uint32 GetTileValue(Map* map, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY)
{
	uint32 tileChunkValue = 0;

	if (tileChunk)
		tileChunkValue = GetTileValueUnchecked(tileChunk, map->ChunkDimension, testTileX, testTileY);

	return tileChunkValue;
}


 void RecanonicalizeCoord(Map* map, uint32* tile, real32* tileRelative)
{
	// Map is toroidal
	int32 offset = RoundReal32ToInt32((*tileRelative) / map->TileSideInMeters);

	*tile += offset;
	*tileRelative -= offset * map->TileSideInMeters;

	Assert(*tileRelative >= (-0.5f * map->TileSideInMeters));
	Assert(*tileRelative <= (0.5f * map->TileSideInMeters));
}

 TileChunkPosition GetTileChunkPosition(Map* map, uint32 absTileX, uint32 absTileY)
{
	TileChunkPosition result = {};

	result.TileChunkX = absTileX >> map->ChunkShift;
	result.TileChunkY = absTileY >> map->ChunkShift;

	result.RelativeTileX = absTileX & map->ChunkMask;
	result.RelativeTileY = absTileY & map->ChunkMask;

	return result;
}

 uint32 GetTileValue(Map* map, uint32 absTileX, uint32 absTileY)
{
	TileChunkPosition chunkPosition = GetTileChunkPosition(map, absTileX, absTileY);

	TileChunk* tileChunk = GetTileChunk(map, chunkPosition.TileChunkX, chunkPosition.TileChunkY);

	uint32 tileValue = GetTileValue(map, tileChunk, chunkPosition.RelativeTileX, chunkPosition.RelativeTileY);

	return tileValue;
}


 TileMapPosition RecanonicalizePosition(Map* map, TileMapPosition position)
{
	TileMapPosition result = position;

	RecanonicalizeCoord(map, &result.AbsTileX, &result.TileRelativeX);
	RecanonicalizeCoord(map, &result.AbsTileY, &result.TileRelativeY);

	return result;
}

bool32 IsMapPointEmpty(Map* map, TileMapPosition position)
{
	uint32 tileValue = GetTileValue(map, position.AbsTileX, position.AbsTileY);

	int32 isEmpty = tileValue == 0;

	return isEmpty;
}

 real32 MetersToPixels(Map* map, int32 meters)
{
	return map->MetersToPixels * meters;
}

 real32 MetersToPixels(Map* map, real32 meters)
{
	return map->MetersToPixels * meters;
}