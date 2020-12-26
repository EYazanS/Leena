#include "Leena.h"

void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
void DrawRectangle(GameScreenBuffer* gameScreenBuffer, real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY, Colour colour);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, World* world, real32 playerWidth, real32 playerHeight);
void DrawTileMap(World* world, GameState* gameState, GameScreenBuffer* screenBuffer, real32 playerX, real32 playerY);

GameAudioBuffer* ReadAudioBufferData(void* memory);
bool32 IsWorldPointEmpty(World* world, WorldPosition position);

inline WorldPosition RecanonicalìzePosition(World* world, WorldPosition position);
inline void RecanonicalizeCoord(World* world, uint32* tile, real32* tileRelative);
inline real32 MetersToPixels(World* world, int32 meters);
inline real32 MetersToPixels(World* world, real32 meters);
inline TileChunkPosition GetTileChunkPosition(World* world, uint32 absTileX, uint32 absTileY);
inline TileChunk* GetTileChunk(World* world, uint32 tileChunkX, uint32 tileChunkY);
inline uint32 GetTileValue(World* world, uint32 absTileX, uint32 absTileY);
inline uint32 GetTileValue(World* world, TileChunk* tileChunk, uint32 testTileX, uint32 testTileY);
inline int32 GetTileValueUnchecked(TileChunk* tileChunk, uint32 tileCountX, uint32 tileX, uint32 tileY);

void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
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

	world.TileChunkCountX = 1;
	world.TileChunkCountY = 1;

	TileChunk tileChunk = { (uint32*)tempTiles };

	world.TileChunks = &tileChunk;
	world.ChunkDimension = 256;

	world.TileSideInMeters = 1.4f;
	world.TileSideInPixels = 60;

	// for using 256x256 tile chunks
	world.ChunkShift = 8;
	world.ChunkMask = (1 << world.ChunkShift) - 1;

	world.MetersToPixels = world.TileSideInPixels / world.TileSideInMeters;

	real32 lowerLeftX = -world.TileSideInPixels / 2.0f;
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

	newPlayerPosition = RecanonicalìzePosition(&world, newPlayerPosition);

	WorldPosition playerLeftPosition = newPlayerPosition;
	playerLeftPosition.TileRelativeX -= 0.5f * playerWidth;
	playerLeftPosition = RecanonicalìzePosition(&world, playerLeftPosition);

	WorldPosition playerRightPosition = newPlayerPosition;
	playerRightPosition.TileRelativeX += 0.5f * playerWidth;
	playerRightPosition = RecanonicalìzePosition(&world, playerRightPosition);

	if (IsWorldPointEmpty(&world, newPlayerPosition) && IsWorldPointEmpty(&world, playerLeftPosition) && IsWorldPointEmpty(&world, playerRightPosition))
		gameState->PlayerPosition = newPlayerPosition;

	DrawTileMap(&world, gameState, screenBuffer, MetersToPixels(&world, gameState->PlayerPosition.TileRelativeX), MetersToPixels(&world, gameState->PlayerPosition.TileRelativeY));

	RenderPlayer(screenBuffer, &world, MetersToPixels(&world, playerWidth), MetersToPixels(&world, playerHeight));

	FillAudioBuffer(thread, gameMemory, soundBuffer);
}

inline void RecanonicalizeCoord(World* world, uint32* tile, real32* tileRelative)
{
	// World is toroidal
	int32 offset = FloorReal32ToInt32((*tileRelative) / world->TileSideInMeters);

	*tile += offset;
	*tileRelative -= offset * world->TileSideInMeters;

	Assert(*tileRelative >= 0);
	Assert(*tileRelative < world->TileSideInMeters);
}

WorldPosition RecanonicalìzePosition(World* world, WorldPosition position)
{
	WorldPosition result = position;

	RecanonicalizeCoord(world, &result.AbsTileX, &result.TileRelativeX);
	RecanonicalizeCoord(world, &result.AbsTileY, &result.TileRelativeY);

	return result;
}

inline TileChunkPosition GetTileChunkPosition(World* world, uint32 absTileX, uint32 absTileY)
{
	TileChunkPosition result = {};

	result.TileChunkX = absTileX >> world->ChunkShift;
	result.TileChunkY = absTileY >> world->ChunkShift;

	result.RelativeTileX = absTileX & world->ChunkMask;
	result.RelativeTileY = absTileY & world->ChunkMask;

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
		tileChunkValue = GetTileValueUnchecked(tileChunk, world->ChunkDimension, testTileX, testTileY);

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

void RenderPlayer(GameScreenBuffer* screenBuffer, World* world, real32 playerWidth, real32 playerHeight)
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
	real32 centerX = 0.5f * (real32)screenBuffer->Width;
	real32 centerY = 0.5f * screenBuffer->Height;

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
				colour = { 0.0f, 0.0f ,0.0f };

			real32 minX = centerX - playerX + ((real32)relativeColumn * world->TileSideInPixels);
			real32 maxX = minX + world->TileSideInPixels;

			real32 minY = centerY + playerY - ((real32)relativeRow * world->TileSideInPixels);
			real32 maxY = minY - world->TileSideInPixels;

			DrawRectangle(screenBuffer, minX, maxY, maxX, minY, colour);
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

	if (tileChunkX <= world->TileChunkCountX && tileChunkY <= world->TileChunkCountY)
		tileChunk = &world->TileChunks[tileChunkY * world->TileChunkCountX + tileChunkX];

	return tileChunk;
}

inline real32 MetersToPixels(World* world, int32 meters)
{
	return world->MetersToPixels * meters;
}

inline real32 MetersToPixels(World* world, real32 meters)
{
	return world->MetersToPixels * meters;
}