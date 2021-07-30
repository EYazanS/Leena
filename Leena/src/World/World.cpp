#include "World.h"

void initializeWorld(World* world)
{
	if (world)
	{
		world->TileSideInMeters = 1.4f;

		world->ChunkSideInMeters = TilesPerChunk * world->TileSideInMeters;

		world->FirstFreeBlock = 0;

		for (u32 tileChunkIndex = 0; tileChunkIndex < ArrayCount(world->TileChunkHash); tileChunkIndex++)
		{
			world->TileChunkHash[tileChunkIndex].X = 0;
			world->TileChunkHash[tileChunkIndex].FirstBlock.EntitiesCount = 0;
		}
	}
}

/// <summary>
/// Recalculate where the position should be for the new coordinat, either X or Y side
/// </summary>
/// <param name="world">current world to recalculate for</param>
/// <param name="coordinate">the current coordinate that will be recalculated</param>
/// <param name="tileRelative">Where relatively are we to the coordinate</param>
void RecanonicalizeCoordinant(World* world, i32* coordinate, r32* coordRelative)
{
	// Offset from the current tile center, if its above 1 then it means we moved one tile
	i32 Offset = RoundReal32ToInt32(*coordRelative / world->ChunkSideInMeters);
	*coordinate += Offset;
	*coordRelative -= (Offset * world->ChunkSideInMeters);

	// Assert(IsCanonical(world, *coordRelative));
}

WorldPosition MapIntoChunkSpace(World* world, WorldPosition basePosition, V2 offset)
{
	WorldPosition result = basePosition;
	result.Offset += offset;

	RecanonicalizeCoordinant(world, &result.X, &result.Offset.X);
	RecanonicalizeCoordinant(world, &result.Y, &result.Offset.Y);


	return result;
}