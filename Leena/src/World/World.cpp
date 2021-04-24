#include "World.h"

void initializeWorld(World* world)
{
	if (world)
	{
		world->ChunkDim = 16;
		world->TileSideInMeters = 1.4f;

		for (u32 tileChunkIndex = 0; tileChunkIndex < ArrayCount(world->TileChunkHash); tileChunkIndex++)
		{
			world->TileChunkHash[tileChunkIndex].X = 0;
		}
	}
}

WorldChunk GetChunkPosition(World* world, u32 x, u32 y, u32 z)
{
	WorldChunk result = {};

	result.X = x >> world->ChunkShift;
	result.Y = y >> world->ChunkShift;
	result.Z = z;

	result.X = x >> world->ChunkShift;
	result.Y = y >> world->ChunkShift;

	return result;
}

/// <summary>
/// Recalculate where the position should be for the new coordinat, either X or Y side
/// </summary>
/// <param name="map">current map to recalculate for</param>
/// <param name="tile">the current tile that will be recalculated</param>
/// <param name="tileRelative">Where relatively are we to the tile</param>
void RecanonicalizeCoordinant(World* world, i32* coordinate, r32* coordRelative)
{
	// Map is toroidal, so it can wrap

	// Offset from the current tile center, if its above 1 then it means we moved one tile
	i32 Offset = RoundReal32ToInt32(*coordRelative / world->TileSideInMeters);
	*coordinate += Offset;
	*coordRelative -= Offset * world->TileSideInMeters;

	Assert(*coordRelative >= (-0.5f * world->TileSideInMeters));
	Assert(*coordRelative <= (0.5f * world->TileSideInMeters));
}

WorldPosition MapIntoWorldSpace(World* map, WorldPosition basePosition, V2 offset)
{
	WorldPosition result = basePosition;
	result.Offset += offset;

	RecanonicalizeCoordinant(map, &result.X, &result.Offset.X);
	RecanonicalizeCoordinant(map, &result.Y, &result.Offset.Y);

	return result;
}

b32 AreOnSameTile(WorldPosition position1, WorldPosition position2)
{
	b32 result = position1.X == position2.X && position1.Y == position2.Y && position1.Z == position2.Z;

	return result;
}