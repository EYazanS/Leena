#include "Map.h"

void initializeMap(MemoryPool* pool, Map* map)
{
	if (map)
	{
		map->TileCountX = 5000;
		map->TileCountY = 5000;
		map->TileCountZ = 3;
		u32 tileCount = map->TileCountX * map->TileCountY * map->TileCountZ;

		map->TileSideInMeters = 1.4f;
	}
}

/// <summary>
/// Recalculate where the position should be for the new coordinat, either X or Y side
/// </summary>
/// <param name="map">current map to recalculate for</param>
/// <param name="tile">the current tile that will be recalculated</param>
/// <param name="tileRelative">Where relatively are we to the tile</param>
void RecanonicalizeCoordinant(Map* map, i32* tile, r32* tileRelative)
{
	// Map is toroidal, so it can wrap

	// Offset from the current tile center, if its above 1 then it means we moved one tile
	i32 Offset = RoundReal32ToInt32(*tileRelative / map->TileSideInMeters);
	*tile += Offset;
	*tileRelative -= Offset * map->TileSideInMeters;

	Assert(*tileRelative >= (-0.5f * map->TileSideInMeters));
	Assert(*tileRelative <= (0.5f * map->TileSideInMeters));
}

MapPosition MapIntoTileSpace(Map* map, MapPosition basePosition, V2 offset)
{
	MapPosition result = basePosition;
	result.Offset += offset;

	RecanonicalizeCoordinant(map, &result.X, &result.Offset.X);
	RecanonicalizeCoordinant(map, &result.Y, &result.Offset.Y);

	return result;
}

b32 AreOnSameTile(MapPosition position1, MapPosition position2)
{
	b32 result = position1.X == position2.X && position1.Y == position2.Y && position1.Z == position2.Z;

	return result;
}