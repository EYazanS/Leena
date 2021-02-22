#include "Leena.h"

void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
void DrawRectangle(GameScreenBuffer* gameScreenBuffer, real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY, Colour colour);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, real32 playerWidth, real32 playerHeight);
void DrawTileMap(World* world, GameState* gameState, GameScreenBuffer* screenBuffer, real32 playerX, real32 playerY);

GameAudioBuffer* ReadAudioBufferData(void* memory);
bool32 IsWorldPointEmpty(World* world, WorldPosition position);

inline WorldPosition RecanonicalizePosition(World* world, WorldPosition position);
inline void RecanonicalizeCoord(World* world, uint32* tile, real32* tileRelative);
inline real32 MetersToPixels(World* world, int32 meters);
inline real32 MetersToPixels(World* world, real32 meters);
inline TileChunkPosition GetTileChunkPosition(World* world, uint32 absTileX, uint32 absTileY);
inline TileChunk* GetTileChunk(World* world, uint32 tileChunkX, uint32 tileChunkY);
inline uint32 GetTileValue(World* world, uint32 absTileX, uint32 absTileY);
inline uint32 GetTileValue(World* world, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY);
inline int32 GetTileValueUnchecked(TileChunk* tileChunk, uint32 tileCountX, uint32 tileX, uint32 tileY);

DllExport void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	if (!gameMemory->IsInitialized)
	{
		gameState->PlayerPosition.AbsTileX = 3;
		gameState->PlayerPosition.AbsTileY = 3;

		gameState->PlayerPosition.TileRelativeX = 0;
		gameState->PlayerPosition.TileRelativeY = 0;

		gameMemory->IsInitialized = true;
	}

	uint32 tempTiles[256][256] =
	{
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },

		{ 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	};

	World world = {};

	world.Map.TileChunkCountX = 1;
	world.Map.TileChunkCountY = 1;

	TileChunk tileChunk = { (uint32*)tempTiles };

	world.Map.TileChunks = &tileChunk;
	world.Map.ChunkDimension = 256;

	world.Map.TileSideInMeters = 1.4f;
	world.Map.TileSideInPixels = 60;

	// for using 256x256 tile chunks
	world.Map.ChunkShift = 8;
	world.Map.ChunkMask = (1 << world.Map.ChunkShift) - 1;

	world.Map.MetersToPixels = world.Map.TileSideInPixels / world.Map.TileSideInMeters;

	real32 lowerLeftX = -world.Map.TileSideInPixels / 2.0f;
	real32 lowerLeftY = (real32)screenBuffer->Height;

	// In Meters
	real32 playerHeight = 1.25f;
	real32 playerWidth = 0.75f * playerHeight;

	real32 playerMovementX = 0.f; // pixels/second
	real32 playerMovementY = 0.f; // pixels/second

	if (input->Keyboard.A.EndedDown)
		playerMovementX -= 1.f;

	if (input->Keyboard.D.EndedDown)
		playerMovementX += 1.f;

	if (input->Keyboard.W.EndedDown)
		playerMovementY += 1.f;

	if (input->Keyboard.S.EndedDown)
		playerMovementY -= 1.f;

	playerMovementY *= 5;
	playerMovementX *= 5;

	//// TODO: Deal with controller movement
	//for (const GameControllerInput& controller : input->Controllers)
	//{
	//	if (controller.IsConnected && controller.IsAnalog)
	//	{
	//		gameState->PlayerPosition.TileRelativeX += static_cast<int>(pixelsToMovePerSec * input->TimeToAdvance * controller.LeftStickAverageX);
	//		gameState->PlayerPosition.TileRelativeY += static_cast<int>(pixelsToMovePerSec * input->TimeToAdvance * controller.LeftStickAverageY);
	//	}
	//}

	WorldPosition newPlayerPosition = gameState->PlayerPosition;

	newPlayerPosition.TileRelativeX += (real32)(input->TimeToAdvance * playerMovementX);
	newPlayerPosition.TileRelativeY += (real32)(input->TimeToAdvance * playerMovementY);

	newPlayerPosition = RecanonicalizePosition(&world, newPlayerPosition);

	WorldPosition playerLeftPosition = newPlayerPosition;

	playerLeftPosition.TileRelativeX -= 0.5f * playerWidth;
	playerLeftPosition = RecanonicalizePosition(&world, playerLeftPosition);

	WorldPosition playerRightPosition = newPlayerPosition;

	playerRightPosition.TileRelativeX += 0.5f * playerWidth;
	playerRightPosition = RecanonicalizePosition(&world, playerRightPosition);

	if (IsWorldPointEmpty(&world, newPlayerPosition) && IsWorldPointEmpty(&world, playerLeftPosition) && IsWorldPointEmpty(&world, playerRightPosition))
		gameState->PlayerPosition = newPlayerPosition;

	DrawTileMap(&world, gameState, screenBuffer, MetersToPixels(&world, gameState->PlayerPosition.TileRelativeX), MetersToPixels(&world, gameState->PlayerPosition.TileRelativeY));

	RenderPlayer(screenBuffer, MetersToPixels(&world, playerWidth), MetersToPixels(&world, playerHeight));

	FillAudioBuffer(thread, gameMemory, soundBuffer);
}

WorldPosition RecanonicalizePosition(World* world, WorldPosition position)
{
	WorldPosition result = position;

	RecanonicalizeCoord(world, &result.AbsTileX, &result.TileRelativeX);
	RecanonicalizeCoord(world, &result.AbsTileY, &result.TileRelativeY);

	return result;
}

inline void RecanonicalizeCoord(World* world, uint32* tile, real32* tileRelative)
{
	// World is toroidal
	int32 offset = RoundReal32ToInt32((*tileRelative) / world->Map.TileSideInMeters);

	*tile += offset;
	*tileRelative -= offset * world->Map.TileSideInMeters;

	Assert(*tileRelative >= (-0.5f * world->Map.TileSideInMeters));
	Assert(*tileRelative <= (0.5f * world->Map.TileSideInMeters));
}

inline TileChunkPosition GetTileChunkPosition(World* world, uint32 absTileX, uint32 absTileY)
{
	TileChunkPosition result = {};

	result.TileChunkX = absTileX >> world->Map.ChunkShift;
	result.TileChunkY = absTileY >> world->Map.ChunkShift;

	result.RelativeTileX = absTileX & world->Map.ChunkMask;
	result.RelativeTileY = absTileY & world->Map.ChunkMask;

	return result;
}

bool32 IsWorldPointEmpty(World* world, WorldPosition position)
{
	uint32 tileValue = GetTileValue(world, position.AbsTileX, position.AbsTileY);

	int32 isEmpty = tileValue == 0;

	return isEmpty;
}

inline uint32 GetTileValue(World* world, uint32 absTileX, uint32 absTileY)
{
	TileChunkPosition chunkPosition = GetTileChunkPosition(world, absTileX, absTileY);

	TileChunk* tileChunk = GetTileChunk(world, chunkPosition.TileChunkX, chunkPosition.TileChunkY);

	uint32 tileValue = GetTileValue(world, tileChunk, chunkPosition.RelativeTileX, chunkPosition.RelativeTileY);

	return tileValue;
}

inline uint32 GetTileValue(World* world, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY)
{
	uint32 tileChunkValue = 0;

	if (tileChunk)
		tileChunkValue = GetTileValueUnchecked(tileChunk, world->Map.ChunkDimension, testTileX, testTileY);

	return tileChunkValue;
}

inline int32 GetTileValueUnchecked(TileChunk* tileChunk, uint32 tileCountX, uint32 tileX, uint32 tileY)
{
	return tileChunk->Tiles[tileY * tileCountX + tileX];
}

void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer)
{
	if (0)
	{
		DebugFileResult file = gameMemory->ReadFile(thread, "resources/Water_Splash_SeaLion_Fienup_001.wav");

		auto result = ReadAudioBufferData(file.Memory);

		*soundBuffer = *result;
	}
}

GameAudioBuffer* ReadAudioBufferData(void* memory)
{
	uint8* byte = (uint8*)memory; // Get the firs 4 bytes of the memory

	// Move to position 21
	byte += 20;

	GameAudioBuffer* result = (GameAudioBuffer*)byte;

	// Move to Data chunk
	char* characterToRead = ((char*)byte);

	characterToRead += sizeof(GameAudioBuffer);

	while (*characterToRead != 'd' || *(characterToRead + 1) != 'a' || *(characterToRead + 2) != 't' || *(characterToRead + 3) != 'a')
	{
		characterToRead++;
	}

	characterToRead += 4;

	byte = (uint8*)(characterToRead);

	uint32 bufferSize = *((uint32*)byte);

	byte += 4;

	// Get one third of a sec
	uint32 totalBytesNeeded = (uint32)(result->SamplesPerSec * 0.5f);

	result->BufferSize = totalBytesNeeded;
	result->BufferData = byte;

	return result;
}

/// <summary>
/// Put the player sprite in the screen buffer for rendering
/// </summary>
/// <param name="screenBuffer">The screen buffer that the function will fill</param>
/// <param name="playerWidth">Player width in pixels</param>
/// <param name="playerHeight">Player height in pixels</param>
void RenderPlayer(GameScreenBuffer* screenBuffer, real32 playerWidth, real32 playerHeight)
{
	Colour colour = { 1.f, 0.f, 1.f };

	real32 centerX = 0.5f * (real32)screenBuffer->Width;
	real32 centerY = 0.5f * screenBuffer->Height;

	real32 playerLeft = centerX - (0.5f * playerWidth);
	real32 playerTop = centerY - playerHeight;

	DrawRectangle(screenBuffer, playerLeft, playerTop, playerLeft + playerWidth, playerTop + playerHeight, colour);
}

void DrawTileMap(World* world, GameState* gameState, GameScreenBuffer* screenBuffer, real32 playerX, real32 playerY)
{
	real32 screenCenterX = 0.5f * (real32)screenBuffer->Width;
	real32 screenCenterY = 0.5f * screenBuffer->Height;

	for (int32 relativeRow = -10; relativeRow < 10; relativeRow++)
	{
		for (int32 relativeColumn = -20; relativeColumn < 20; relativeColumn++)
		{
			uint32 column = relativeColumn + gameState->PlayerPosition.AbsTileX;
			uint32 row = relativeRow + gameState->PlayerPosition.AbsTileY;

			Colour colour = {};

			(GetTileValue(world, column, row) == 1) ? colour = { 1.f, 1.f, 1.f } : colour = { 0.7f, 0.7f, 0.7f };

			// For debug
			if (column == gameState->PlayerPosition.AbsTileX && row == gameState->PlayerPosition.AbsTileY)
				colour = { 0.0f, 0.0f, 0.0f };

			/// TODO: Fix why the tile map y is inverted, like it goes out of boundries from down instead of up
			real32 tileCenterX = screenCenterX - playerX + ((real32)relativeColumn * world->Map.TileSideInPixels);
			real32 tileCenterY = screenCenterY + playerY - ((real32)relativeRow * world->Map.TileSideInPixels);

			real32 tileMinX = tileCenterX - (0.5f * world->Map.TileSideInPixels);
			real32 tileMinY = tileCenterY - (0.5f * world->Map.TileSideInPixels);

			real32 tileMaxX = tileCenterX + (0.5f * world->Map.TileSideInPixels);
			real32 tileMaxY = tileMinY - (world->Map.TileSideInPixels);

			DrawRectangle(screenBuffer, tileMinX, tileMaxY, tileMaxX, tileMinY, colour);
		}
	}
}

void DrawRectangle(
	GameScreenBuffer* gameScreenBuffer,
	real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY,
	Colour colour)
{
	int32 minX = RoundReal32ToInt32(realMinX);
	int32 maxX = RoundReal32ToInt32(realMaxX);

	int32 minY = RoundReal32ToInt32(realMinY);
	int32 maxY = RoundReal32ToInt32(realMaxY);

	// Clip to the nearest valid pixel
	if (minX < 0)
		minX = 0;

	if (minY < 0)
		minY = 0;

	if (maxX > gameScreenBuffer->Width)
		maxX = gameScreenBuffer->Width;

	if (maxY > gameScreenBuffer->Height)
		maxY = gameScreenBuffer->Height;

	// Bit Pattern = x0 AA RR GG BB
	uint32 finalColour =
		(RoundReal32ToInt32(colour.Red * 255.f) << 16) |
		(RoundReal32ToInt32(colour.Green * 255.f) << 8) |
		(RoundReal32ToInt32(colour.Blue * 255.f) << 0);

	uint8* endOfBuffer = ((uint8*)gameScreenBuffer->Memory) + ((uint64)gameScreenBuffer->Pitch * (uint64)gameScreenBuffer->Height);

	uint8* row = (((uint8*)gameScreenBuffer->Memory) + (int64)minX * 4 + (int64)minY * gameScreenBuffer->Pitch);

	for (int32 y = minY; y < maxY; ++y)
	{
		uint32* pixel = (uint32*)row;

		for (int32 x = minX; x < maxX + 10; ++x)
		{
			*pixel++ = finalColour;
		}

		row += gameScreenBuffer->Pitch;
	}
}

inline TileChunk* GetTileChunk(World* world, uint32 tileChunkX, uint32 tileChunkY)
{
	TileChunk* tileChunk = 0;

	if (tileChunkX <= world->Map.TileChunkCountX && tileChunkY <= world->Map.TileChunkCountY)
		tileChunk = &world->Map.TileChunks[tileChunkY * world->Map.TileChunkCountX + tileChunkX];

	return tileChunk;
}

inline real32 MetersToPixels(World* world, int32 meters)
{
	return world->Map.MetersToPixels * meters;
}

inline real32 MetersToPixels(World* world, real32 meters)
{
	return world->Map.MetersToPixels * meters;
}