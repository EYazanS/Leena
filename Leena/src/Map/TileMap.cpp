#include "TileMap.h"

void initializeMap(MemoryPool* pool, Map* map)
{
	if (map)
	{
		map->TileCountX = 5000;
		map->TileCountY = 5000;
		map->TileCountZ = 3;
		uint32 tileCount = map->TileCountX * map->TileCountY * map->TileCountZ;
		map->Tiles = PushArray(pool, tileCount, TileValue);
		map->TileSideInMeters = 1.4f;
	}
}

/// <summary>
/// Recalculate where the position should be for the new coordinat, either X or Y side
/// </summary>
/// <param name="map">current map to recalculate for</param>
/// <param name="tile">the current tile that will be recalculated</param>
/// <param name="tileRelative">Where relatively are we to the tile</param>
void RecanonicalizeCoordinant(Map* map, uint32* tile, real32* tileRelative)
{
	// Map is toroidal, so it can wrap

	// Offset from the current tile center, if its above 1 then it means we moved one tile
	int32 Offset = RoundReal32ToInt32(*tileRelative / map->TileSideInMeters);
	*tile += Offset;
	*tileRelative -= Offset * map->TileSideInMeters;

	Assert(*tileRelative >= (-0.5f * map->TileSideInMeters));
	Assert(*tileRelative <= (0.5f * map->TileSideInMeters));
}

MapPosition RecanonicalizePosition(Map* map, MapPosition position)
{
	MapPosition result = position;

	RecanonicalizeCoordinant(map, &result.X, &result.TileRelativeX);
	RecanonicalizeCoordinant(map, &result.Y, &result.TileRelativeY);

	return result;
}

TileValue GetTileValue(Map* map, uint32 x, uint32 y, uint32 z)
{
	TileValue tileValue = TileValue::Invalid;

	if ((x >= 0) && (x < map->TileCountX) &&
		(y >= 0) && (y < map->TileCountY) &&
		(z >= 0) && (z < map->TileCountZ))
	{
		tileValue = map->Tiles[(z * map->TileCountY * map->TileCountX) + (y * map->TileCountX) + x];
	}

	return tileValue;
}

TileValue GetTileValue(Map* map, MapPosition position)
{
	TileValue tileValue = GetTileValue(map, position.X, position.Y, position.Z);

	return tileValue;
}

bool32 IsMapPointEmpty(Map* map, MapPosition position)
{
	TileValue tileValue = GetTileValue(map, position.X, position.Y, position.Z);

	int32 isEmpty = tileValue == TileValue::Empty || tileValue == TileValue::DoorUp || tileValue == TileValue::DoorDown;

	return isEmpty;
}

void SetTileValue(Map* map, uint32 x, uint32 y, uint32 z, TileValue value)
{
	if ((x >= 0) && (x < map->TileCountX) &&
		(y >= 0) && (y < map->TileCountY) &&
		(z >= 0) && (z < map->TileCountZ))
	{
		map->Tiles[(z * map->TileCountY * map->TileCountX) + (y * map->TileCountX) + x] = value;
	}
}

bool32 AreOnSameTile(MapPosition position1, MapPosition position2)
{
	bool32 result = position1.X == position2.X && position1.Y == position2.Y && position1.Z == position2.Z;

	return result;
}