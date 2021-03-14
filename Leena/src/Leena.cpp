#include "Leena.h"

void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
void DrawRectangle(GameScreenBuffer* gameScreenBuffer, real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY, Colour colour);

GameAudioBuffer* ReadAudioBufferData(void* memory);

DllExport void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermanentStorage;

	if (!gameMemory->IsInitialized)
	{
		gameState->PlayerPosition.AbsTileX = 1;
		gameState->PlayerPosition.AbsTileY = 4;
		gameState->PlayerPosition.TileRelativeX = 0.0f;
		gameState->PlayerPosition.TileRelativeY = 0.0f;

		InitilizePool(&gameState->WorldMemoryPool, gameMemory->PermanentStorageSize - sizeof(GameState), (uint8*)gameMemory->PermanentStorage + sizeof(GameState));

		gameState->World = PushSize(&gameState->WorldMemoryPool, World);
		World* world = gameState->World;
		world->Map = PushSize(&gameState->WorldMemoryPool, Map);

		Map* map = world->Map;

		map->ChunkShift = 4;
		map->ChunkMask = (1 << map->ChunkShift) - 1;
		map->ChunkDimension = (1 << map->ChunkShift);

		map->TileChunkCountX = 128;
		map->TileChunkCountY = 128;
		map->TileChunkCountZ = 2;

		map->TileChunks = PushArray(&gameState->WorldMemoryPool, (uint64)map->TileChunkCountX * (uint64)map->TileChunkCountY * (uint64)map->TileChunkCountZ, TileChunk);

		map->TileSideInMeters = 1.4f;

		uint32 sandomNumberIndex = 0;

		uint32 tilesPerWidth = 17;
		uint32 tilesPerHeight = 9;
		uint32 screenX = 0;
		uint32 screenY = 0;
		uint32 absTileZ = 0;

		bool32 doorLeft = false;
		bool32 doorRight = false;
		bool32 doorTop = false;
		bool32 doorBottom = false;
		bool32 doorUp = false;
		bool32 doorDown = false;

		for (uint32 screenIndex = 0; screenIndex < 100; ++screenIndex)
		{
			Assert(sandomNumberIndex < ArrayCount(randomNumberTable));

			uint32 randomChoice;

			if (doorUp || doorDown)
			{
				randomChoice = randomNumberTable[sandomNumberIndex++] % 2;
			}
			else
			{
				randomChoice = randomNumberTable[sandomNumberIndex++] % 3;
			}

			if (randomChoice == 2)
			{
				if (absTileZ == 0)
				{
					doorUp = true;
				}
				else
				{
					doorDown = true;
				}
			}
			else if (randomChoice == 1)
			{
				doorRight = true;
			}
			else
			{
				doorTop = true;
			}

			for (uint32 tileY = 0; tileY < tilesPerHeight; ++tileY)
			{
				for (uint32 tileX = 0; tileX < tilesPerWidth; ++tileX)
				{
					uint32 absTileX = screenX * tilesPerWidth + tileX;
					uint32 absTileY = screenY * tilesPerHeight + tileY;

					uint32 tileValue = 1;

					if (tileX == 0 && (!doorLeft || (tileY != (tilesPerHeight / 2))))
					{
						tileValue = 2;
					}

					if ((tileX == (tilesPerWidth - 1)) && (!doorRight || (tileY != (tilesPerHeight / 2))))
					{
						tileValue = 2;
					}

					if (tileY == 0 && (!doorBottom || (tileX != (tilesPerWidth / 2))))
					{
						tileValue = 2;
					}

					if ((tileY == (tilesPerHeight - 1)) && (!doorTop || (tileX != (tilesPerWidth / 2))))
					{
						tileValue = 2;
					}

					if (tileX == 10 && tileY == 6)
					{
						if (doorUp)
						{
							tileValue = 3;
						}

						if (doorDown)
						{
							tileValue = 4;
						}
					}

					SetTileValue(&gameState->WorldMemoryPool, world->Map, absTileX, absTileY, absTileZ, tileValue);
				}
			}

			doorLeft = doorRight;

			doorBottom = doorTop;

			if (doorUp)
			{
				doorDown = true;
				doorUp = false;
			}
			else if (doorDown)
			{
				doorUp = true;
				doorDown = false;
			}
			else
			{
				doorUp = false;
				doorDown = false;
			}

			doorRight = false;
			doorTop = false;

			if (randomChoice == 2)
			{
				if (absTileZ == 0)
				{
					absTileZ = 1;
				}
				else
				{
					absTileZ = 0;
				}

			}
			else if (randomChoice == 1)
			{
				screenX++;
			}
			else
			{
				screenY++;
			}
		}

		gameMemory->IsInitialized = true;
	}

	World* world = gameState->World;
	Map* map = gameState->World->Map;

	// In Meters
	real32 playerHeight = 1.4f;
	real32 playerWidth = 0.75f * playerHeight;

	// Handle input
	for (uint8 controllerIndex = 0; controllerIndex < ArrayCount(input->Controllers); ++controllerIndex)
	{
		GameControllerInput* controller = GetController(input, controllerIndex);

		if (controller->IsAnalog)
		{
			//NOTE: Use analog movement tuning
		}
		else
		{
			//NOTE: Use digital movement tuning
			real32 dPlayerX = 0.0f;
			real32 dPlayerY = 0.0f;

			if (controller->MoveUp.EndedDown)
			{
				dPlayerY = 1.0f;
			}
			if (controller->MoveDown.EndedDown)
			{
				dPlayerY = -1.0f;
			}
			if (controller->MoveLeft.EndedDown)
			{
				dPlayerX = -1.0f;
			}
			if (controller->MoveRight.EndedDown)
			{
				dPlayerX = 1.0f;
			}

			real32 PlayerSpeed = 5.0f;

			if (controller->X.EndedDown)
			{
				PlayerSpeed = 30.0f;
			}

			if (controller->Start.EndedDown)
			{
				gameMemory->IsInitialized = false;
			}

			dPlayerX *= PlayerSpeed;
			dPlayerY *= PlayerSpeed;

			TileMapPosition newPlayerPosition = gameState->PlayerPosition;
			newPlayerPosition.TileRelativeX += (real32)input->TimeToAdvance * dPlayerX;
			newPlayerPosition.TileRelativeY += (real32)input->TimeToAdvance * dPlayerY;
			newPlayerPosition = RecanonicalizePosition(map, newPlayerPosition);

			TileMapPosition playerLeft = newPlayerPosition;
			playerLeft.TileRelativeX -= 0.5f * playerWidth;
			playerLeft = RecanonicalizePosition(map, playerLeft);

			TileMapPosition playerRight = newPlayerPosition;
			playerRight.TileRelativeX += 0.5f * playerWidth;
			playerRight = RecanonicalizePosition(map, playerRight);


			if (IsMapPointEmpty(map, newPlayerPosition) &&
				IsMapPointEmpty(map, playerLeft) &&
				IsMapPointEmpty(map, playerRight))
			{
				gameState->PlayerPosition = newPlayerPosition;
			}
		}
	}

	int32 TileSideInPixels = 60;
	real32 MetersToPixels = (real32)TileSideInPixels / (real32)map->TileSideInMeters;

	DrawRectangle(screenBuffer, 0, 0, (real32)screenBuffer->Width, (real32)screenBuffer->Height, { 1.0f, 0.0f, 0.0f });

	real32 screenCenterX = 0.5f * (real32)screenBuffer->Width;
	real32 screenCenterY = 0.5f * (real32)screenBuffer->Height;

	for (int32 relRow = -10; relRow < 10; ++relRow)
	{
		for (int32 relColumn = -20; relColumn < 20; ++relColumn)
		{
			uint32 column = gameState->PlayerPosition.AbsTileX + relColumn;
			uint32 row = gameState->PlayerPosition.AbsTileY + relRow;
			uint32 floor = gameState->PlayerPosition.AbsTileZ;

			uint32 tileValue = GetTileValue(map, column, row, floor);

			if (tileValue > 0)
			{
				real32 colour = 0.5f;

				if (tileValue == 2)
				{
					colour = 1.0f;
				}

				if (tileValue > 2)
				{
					colour = 0.25f;
				}

				if ((column == gameState->PlayerPosition.AbsTileX) &&
					(row == gameState->PlayerPosition.AbsTileY))
				{
					colour = 0;
				}

				real32 centerX = (screenCenterX - MetersToPixels * gameState->PlayerPosition.TileRelativeX) + ((real32)relColumn) * TileSideInPixels;
				real32 centerY = (screenCenterY + MetersToPixels * gameState->PlayerPosition.TileRelativeY) - ((real32)relRow) * TileSideInPixels;
				real32 minX = centerX - 0.5f * TileSideInPixels;
				real32 minY = centerY - 0.5f * TileSideInPixels;
				real32 maxX = centerX + 0.5f * TileSideInPixels;
				real32 maxY = centerY + 0.5f * TileSideInPixels;

				DrawRectangle(screenBuffer, minX, minY, maxX, maxY, { colour, colour, colour });
			}
		}
	}

	real32 playerR = 1.0f;
	real32 playerG = 1.0f;
	real32 playerB = 0.0f;

	real32 PlayerLeft = screenCenterX - 0.5f * MetersToPixels * playerWidth;
	real32 PlayerTop = screenCenterY - MetersToPixels * playerHeight;

	DrawRectangle(screenBuffer, PlayerLeft, PlayerTop,
		PlayerLeft + MetersToPixels * playerWidth,
		PlayerTop + MetersToPixels * playerHeight,
		{ playerR, playerG, playerB });
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

	uint8* row = (((uint8*)gameScreenBuffer->Memory) + (int64)minX * gameScreenBuffer->BytesPerPixel + (int64)minY * gameScreenBuffer->Pitch);

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