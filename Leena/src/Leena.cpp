#include "Leena.h"

void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
void DrawRectangle(GameScreenBuffer* gameScreenBuffer, real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY, Colour colour);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, real32 playerWidth, real32 playerHeight);
void DrawTileMap(World* world, GameState* gameState, GameScreenBuffer* screenBuffer, real32 playerX, real32 playerY);
GameAudioBuffer* ReadAudioBufferData(void* memory);

#define PushArray(pool, size, type) (type*) PushSize_(pool, size +  sizeof(type))
#define PushSize(pool, type) (type*) PushSize_(pool, sizeof(type))

void* PushSize_(MemoryPool* pool, MemorySizeIndex size);
void InitilizePool(MemoryPool* pool, MemorySizeIndex size, uint8* storage);

DllExport void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	if (!gameMemory->IsInitialized)
	{
		gameState->PlayerPosition.AbsTileX = 3;
		gameState->PlayerPosition.AbsTileY = 5;

		gameState->PlayerPosition.TileRelativeX = 0;
		gameState->PlayerPosition.TileRelativeY = 0;

		InitilizePool(&gameState->WorldMemoryPool, gameMemory->PermenantStorageSize - sizeof(GameState), (uint8*)gameMemory->PermenantStorage + sizeof(GameState));

		gameState->World = PushSize(&gameState->WorldMemoryPool, World);

		World* world = gameState->World;

		world->Map = PushSize(&gameState->WorldMemoryPool, Map);

		world->Map->TileChunkCountX = 16;
		world->Map->TileChunkCountY = 16;

		world->Map->TileChunks = PushArray(&gameState->WorldMemoryPool, static_cast<uint64>(world->Map->TileChunkCountX) * static_cast<uint64>(world->Map->TileChunkCountY), TileChunk);
		world->Map->TileSideInMeters = 1.4f;

		world->Map->TileSideInPixels = 60;

		// For using 256 x 256 tile chunks
		world->Map->ChunkShift = 8;
		world->Map->ChunkMask = (1 << world->Map->ChunkShift) - 1;
		world->Map->ChunkDimension = (1 << world->Map->ChunkShift);

		for (uint32 y = 0; y < world->Map->TileChunkCountY; y++)
		{
			for (uint32 x = 0; x < world->Map->TileChunkCountX; x++)
			{
				world->Map->TileChunks[y * world->Map->TileChunkCountX + x].Tiles = PushArray(&gameState->WorldMemoryPool, static_cast<uint64>(world->Map->ChunkDimension) * static_cast<uint64>(world->Map->ChunkDimension), uint32);
			}
		}

		world->Map->MetersToPixels = world->Map->TileSideInPixels / world->Map->TileSideInMeters;

		uint32 tilerPerScreenWidth = 17;
		uint32 tilerPerScreenHeight = 9;

		for (uint32 screenY = 0; screenY < 32; screenY++)
		{
			for (uint32 screenX = 0; screenX < 32; screenX++)
			{
				for (uint32 tileY = 0; tileY < tilerPerScreenHeight; tileY++)
				{
					for (uint32 tileX = 0; tileX < tilerPerScreenWidth; tileX++)
					{
						uint32 absTileX = screenX * tilerPerScreenWidth + tileX;
						uint32 absTileY = screenY * tilerPerScreenHeight + tileY;
						SetTileValue(&gameState->WorldMemoryPool, world->Map, absTileX, absTileY, tileX == tileY && tileY % 2 == 0);
					}
				}
			}
		}

		gameMemory->IsInitialized = true;
	}

	World* world = gameState->World;

	// In Meters
	real32 playerHeight = 1.4f;
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

	TileMapPosition newPlayerPosition = gameState->PlayerPosition;

	newPlayerPosition.TileRelativeX += (real32)(input->TimeToAdvance * playerMovementX);
	newPlayerPosition.TileRelativeY += (real32)(input->TimeToAdvance * playerMovementY);

	newPlayerPosition = RecanonicalizePosition(world->Map, newPlayerPosition);

	TileMapPosition playerLeftPosition = newPlayerPosition;

	playerLeftPosition.TileRelativeX -= 0.5f * playerWidth;
	playerLeftPosition = RecanonicalizePosition(world->Map, playerLeftPosition);

	TileMapPosition playerRightPosition = newPlayerPosition;

	playerRightPosition.TileRelativeX += 0.5f * playerWidth;
	playerRightPosition = RecanonicalizePosition(world->Map, playerRightPosition);

	if (IsMapPointEmpty(world->Map, newPlayerPosition) && IsMapPointEmpty(world->Map, playerLeftPosition) && IsMapPointEmpty(world->Map, playerRightPosition))
		gameState->PlayerPosition = newPlayerPosition;

	DrawTileMap(world, gameState, screenBuffer, MetersToPixels(world->Map, gameState->PlayerPosition.TileRelativeX), MetersToPixels(world->Map, gameState->PlayerPosition.TileRelativeY));

	RenderPlayer(screenBuffer, MetersToPixels(world->Map, playerWidth), MetersToPixels(world->Map, playerHeight));

	FillAudioBuffer(thread, gameMemory, soundBuffer);
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
	real32 playerTop = centerY - (0.5f * playerHeight);

	DrawRectangle(screenBuffer, playerLeft, playerTop, playerLeft + playerWidth, playerTop + playerHeight, colour);
}

void DrawTileMap(World* world, GameState* gameState, GameScreenBuffer* screenBuffer, real32 playerX, real32 playerY)
{
	DrawRectangle(screenBuffer, 0.0f, 0.0f, (real32)screenBuffer->Width, (real32)screenBuffer->Height, { 0.8f, 0.8f ,0.0f });

	real32 screenCenterX = 0.5f * (real32)screenBuffer->Width;
	real32 screenCenterY = 0.5f * (real32)screenBuffer->Height;

	// Relative to the player position, so its current row -10 and +10
	for (int32 relativeRow = -10; relativeRow < 10; relativeRow++)
	{
		// Relative to the player position, so its current column -20 and +20
		for (int32 relativeColumn = -20; relativeColumn < 20; relativeColumn++)
		{
			uint32 column = relativeColumn + gameState->PlayerPosition.AbsTileX;
			uint32 row = relativeRow + gameState->PlayerPosition.AbsTileY;

			Colour colour = {};

			(GetTileValue(world->Map, column, row) == 1) ? colour = { 1.f, 1.f, 1.f } : colour = { 0.7f, 0.7f, 0.7f };

			// For debug
			if (column == gameState->PlayerPosition.AbsTileX && row == gameState->PlayerPosition.AbsTileY)
				colour = { 0.0f, 0.0f, 0.0f };

			real32 tileCenterX = screenCenterX - playerX + ((real32)relativeColumn * world->Map->TileSideInPixels);
			real32 tileCenterY = screenCenterY + playerY - ((real32)relativeRow * world->Map->TileSideInPixels);

			real32 tileMinX = tileCenterX - (0.5f * world->Map->TileSideInPixels);
			real32 tileMinY = tileCenterY - (0.5f * world->Map->TileSideInPixels);

			real32 tileMaxX = tileCenterX + (0.5f * world->Map->TileSideInPixels);
			real32 tileMaxY = tileCenterY + (0.5f * world->Map->TileSideInPixels);

			DrawRectangle(screenBuffer, tileMinX, tileMinY, tileMaxX, tileMaxY, colour);
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

		for (int32 x = minX; x < maxX; ++x)
		{
			*pixel++ = finalColour;
		}

		row += gameScreenBuffer->Pitch;
	}
}

void InitilizePool(MemoryPool* pool, MemorySizeIndex size, uint8* storage)
{
	pool->Size = size;
	pool->BaseMemory = storage;
	pool->UsedAmount = 0;
}

void* PushSize_(MemoryPool* pool, MemorySizeIndex size)
{
	Assert(pool->UsedAmount + pool->UsedAmount <= pool->Size);
	void* result = pool->BaseMemory + pool->UsedAmount;
	pool->UsedAmount += size;
	return result;
}