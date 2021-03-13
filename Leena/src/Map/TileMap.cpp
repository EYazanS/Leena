#include "TileMap.h"

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
	int32 Offset = RoundReal32ToInt32(*tileRelative / map->TileSideInMeters);
	*tile += Offset;
	*tileRelative -= Offset * map->TileSideInMeters;

	Assert(*tileRelative >= (-0.5f * map->TileSideInMeters));
	Assert(*tileRelative <= (0.5f * map->TileSideInMeters));
}

TileMapPosition RecanonicalizePosition(Map* map, TileMapPosition position)
{
	TileMapPosition result = position;

	RecanonicalizeCoordinant(map, &result.AbsTileX, &result.TileRelativeX);
	RecanonicalizeCoordinant(map, &result.AbsTileY, &result.TileRelativeY);

	return result;
}

TileChunk* GetTileChunk(Map* map, uint32 tileChunkX, uint32 tileChunkY, uint32 tileChunkZ)
{
	TileChunk* tileChunk = 0;

	if ((tileChunkX >= 0) && (tileChunkX < map->TileChunkCountX) &&
		(tileChunkY >= 0) && (tileChunkY < map->TileChunkCountY) &&
		(tileChunkZ >= 0) && (tileChunkZ < map->TileChunkCountZ))
	{
		tileChunk = &map->TileChunks[
			tileChunkZ * map->TileChunkCountY * map->TileChunkCountX +
				tileChunkY * map->TileChunkCountX +
				tileChunkX];

	}

	return tileChunk;
}

int32 GetTileValueUnchecked(Map* map, TileChunk* tileChunk, uint32 tileX, uint32 tileY)
{
	Assert(tileChunk);
	Assert(tileX < map->ChunkDimension);
	Assert(tileY < map->ChunkDimension);

	if (!tileChunk || !map)
		return 0;

	uint32 tileValue = tileChunk->Tiles[tileY * map->ChunkDimension + tileX];

	return tileValue;
}

void SetTileValueUnchecked(Map* map, TileChunk* tileChunk, uint32 tileX, uint32 tileY, uint32 value)
{
	Assert(tileChunk);
	Assert(tileX < map->ChunkDimension);
	Assert(tileY < map->ChunkDimension);

	if (!tileChunk || !map)
		return;

	tileChunk->Tiles[tileY * map->ChunkDimension + tileX] = value;
}

uint32 GetTileValue(Map* map, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY)
{
	uint32 tileChunkValue = 0;

	if (tileChunk && tileChunk->Tiles)
		tileChunkValue = GetTileValueUnchecked(map, tileChunk, testTileX, testTileY);

	return tileChunkValue;
}

void SetTileValue(Map* map, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY, uint32 value)
{
	if (tileChunk && tileChunk->Tiles)
		SetTileValueUnchecked(map, tileChunk, testTileX, testTileY, value);
}

TileChunkPosition GetTileChunkPosition(Map* map, uint32 absTileX, uint32 absTileY, uint32 tileZ)
{
	TileChunkPosition result = {};

	result.TileChunkX = absTileX >> map->ChunkShift;
	result.TileChunkY = absTileY >> map->ChunkShift;

	result.TileChunkZ = tileZ;

	result.RelativeTileX = absTileX & map->ChunkMask;
	result.RelativeTileY = absTileY & map->ChunkMask;

	return result;
}

uint32 GetTileValue(Map* map, uint32 absTileX, uint32 absTileY, uint32 absTileZ)
{
	TileChunkPosition chunkPosition = GetTileChunkPosition(map, absTileX, absTileY, absTileZ);

	TileChunk* tileChunk = GetTileChunk(map, chunkPosition.TileChunkX, chunkPosition.TileChunkY, chunkPosition.TileChunkZ);

	uint32 tileValue = GetTileValue(map, tileChunk, chunkPosition.RelativeTileX, chunkPosition.RelativeTileY);

	return tileValue;
}


bool32 IsMapPointEmpty(Map* map, TileMapPosition position)
{
	uint32 tileValue = GetTileValue(map, position.AbsTileX, position.AbsTileY, position.AbsTileZ);

	int32 isEmpty = tileValue == 1;

	return isEmpty;
}

void SetTileValue(MemoryPool* pool, Map* map, uint32 tileX, uint32 tileY, uint32 tileZ, uint32 value)
{
	TileChunkPosition chunkPosistion = GetTileChunkPosition(map, tileX, tileY, tileZ);

	TileChunk* tileChunk = GetTileChunk(map, chunkPosistion.TileChunkX, chunkPosistion.TileChunkY, chunkPosistion.TileChunkZ);

	Assert(tileChunk);

	if (tileChunk && !tileChunk->Tiles)
	{
		uint32 tileCount = map->ChunkDimension * map->ChunkDimension;

		tileChunk->Tiles = PushArray(pool, tileCount, uint32);

		for (uint32 TileIndex = 0; TileIndex < tileCount; ++TileIndex)
		{
			tileChunk->Tiles[TileIndex] = 1;
		}
	}

	SetTileValue(map, tileChunk, chunkPosistion.RelativeTileX, chunkPosistion.RelativeTileY, value);
}