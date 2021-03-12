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

void SetTileValueUnchecked(TileChunk* tileChunk, uint32 tileCountX, uint32 tileX, uint32 tileY, uint32 value)
{
	tileChunk->Tiles[tileY * tileCountX + tileX] = value;
}

uint32 GetTileValue(Map* map, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY)
{
	uint32 tileChunkValue = 0;

	if (tileChunk && tileChunk->Tiles)
		tileChunkValue = GetTileValueUnchecked(tileChunk, map->ChunkDimension, testTileX, testTileY);

	return tileChunkValue;
}

void SetTileValue(Map* map, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY, uint32 value)
{
	if (tileChunk && tileChunk->Tiles)
		SetTileValueUnchecked(tileChunk, map->ChunkDimension, testTileX, testTileY, value);
}

/// <summary>
/// Recalculate where the position should be for the new coordinat, either X or Y side
/// </summary>
/// <param name="map">current map to recalculate for</param>
/// <param name="tile">the current tile that will be recalculated</param>
/// <param name="tileRelative">Where relatively are we to the tile</param>
void RecanonicalizeCoordinant(Map* map, uint32* tile, real32* tileRelative)
{
	// Map is toroidal

	// Offset from the current tile center, if its above 1 then it means we moved one tile
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

	RecanonicalizeCoordinant(map, &result.AbsTileX, &result.TileRelativeX);
	RecanonicalizeCoordinant(map, &result.AbsTileY, &result.TileRelativeY);

	return result;
}

bool32 IsMapPointEmpty(Map* map, TileMapPosition position)
{
	uint32 tileValue = GetTileValue(map, position.AbsTileX, position.AbsTileY);

	int32 isEmpty = tileValue == 1;

	return isEmpty;
}

void SetTileValue(MemoryPool* pool, Map* map, uint32 tileX, uint32 tileY, uint32 value)
{
	TileChunkPosition chunkPosition = GetTileChunkPosition(map, tileX, tileY);
	TileChunk* chunk = GetTileChunk(map, chunkPosition.TileChunkX, chunkPosition.TileChunkY);

	// TODO: on demand tile chink creation
	Assert(chunk);

	if (chunk && !chunk->Tiles)
	{
		uint32 tilesCount = map->ChunkDimension * map->ChunkDimension;

		chunk->Tiles = PushArray(pool, tilesCount, uint32);

		for (uint32 tileIndex = 0; tileIndex < tilesCount; tileIndex++)
		{
			chunk->Tiles[tileIndex] = 1;
		}
	}

	SetTileValue(map, chunk, chunkPosition.RelativeTileX, chunkPosition.RelativeTileY, value);

}