#include "Leena.h"

// GRAPHICS
LoadedBitmap DebugLoadBmp(ThreadContext* thread, PlatformReadEntireFile* readFile, const char* fileName);
void DrawRectangle(GameScreenBuffer* gameScreenBuffer, Vector2d vecMin, Vector2d vecMax, Colour colour);
void DrawBitmap(LoadedBitmap* bitmap, GameScreenBuffer* screenBuffer, real32 realX, real32 realY, int32 alignX = 0, int32 alignY = 0);

// AUDIO
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer* soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);

// To pack the struct tightly and prevent combiler from 
// aligning the fields 
#pragma pack(push, 1)
struct BitmapHeader
{
	uint16 FileType;
	uint32 FileSize;
	uint16 Reserved1;
	uint16 Reserved2;
	uint32 BitmapOffset;
	uint32 Size;
	int32 Width;
	int32 Height;
	uint16 Planes;
	uint16 BitsPerPixel;
	uint32 Compression;
	uint32 SizeOfBitmap;
	int32 HorzResoloution;
	int32 VertResoloution;
	uint32 ColoursUsed;
	uint32 ColoursImportant;

	uint32 RedMask;
	uint32 GreeenMask;
	uint32 BlueMask;
	uint32 AlphaMask;
};
#pragma pack(pop)

DllExport void GameUpdateAndRender(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermanentStorage;

	if (!gameMemory->IsInitialized)
	{
		gameState->Background = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_background.bmp");

		PlayerBitMap* playerBitMap;
		playerBitMap = gameState->playerBitMaps;

		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_torso.bmp");
		playerBitMap->AlignX = 72;
		playerBitMap->AlignY = 182;

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_torso.bmp");
		playerBitMap->AlignX = 72;
		playerBitMap->AlignY = 182;

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_torso.bmp");
		playerBitMap->AlignX = 72;
		playerBitMap->AlignY = 182;

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_torso.bmp");
		playerBitMap->AlignX = 72;
		playerBitMap->AlignY = 182;

		gameState->CameraPosition.X = 17 / 2;
		gameState->CameraPosition.Y = 9 / 2;
		gameState->CameraPosition.Z = 0;

		gameState->PlayerPosition.X = 1;
		gameState->PlayerPosition.Y = 4;
		gameState->PlayerPosition.Z = 0;

		gameState->PlayerPosition.Offset = {};

		InitilizePool(&gameState->WorldMemoryPool, gameMemory->PermanentStorageSize - sizeof(GameState), (uint8*)gameMemory->PermanentStorage + sizeof(GameState));

		gameState->World = PushSize(&gameState->WorldMemoryPool, World);
		World* world = gameState->World;
		world->Map = PushSize(&gameState->WorldMemoryPool, Map);

		Map* map = world->Map;

		initializeMap(&gameState->WorldMemoryPool, map);

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

			bool32 createdZDoor = false;

			if (randomChoice == 2)
			{
				createdZDoor = true;
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

					TileValue tileValue = TileValue::Empty;

					if (tileX == 0 && (!doorLeft || (tileY != (tilesPerHeight / 2))))
					{
						tileValue = TileValue::Wall;
					}

					if ((tileX == (tilesPerWidth - 1)) && (!doorRight || (tileY != (tilesPerHeight / 2))))
					{
						tileValue = TileValue::Wall;
					}

					if (tileY == 0 && (!doorBottom || (tileX != (tilesPerWidth / 2))))
					{
						tileValue = TileValue::Wall;
					}

					if ((tileY == (tilesPerHeight - 1)) && (!doorTop || (tileX != (tilesPerWidth / 2))))
					{
						tileValue = TileValue::Wall;
					}

					if (tileX == 10 && tileY == 6)
					{
						if (doorUp)
						{
							tileValue = TileValue::DoorUp;
						}

						if (doorDown)
						{
							tileValue = TileValue::DoorDown;
						}
					}

					SetTileValue(world->Map, absTileX, absTileY, absTileZ, tileValue);
				}
			}

			doorLeft = doorRight;

			doorBottom = doorTop;

			if (createdZDoor)
			{
				doorDown = !doorDown;
				doorUp = !doorUp;
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

	MapPosition oldPlayerPosition = gameState->PlayerPosition;
	Vector2d playerAcceleration = {};
	real32 PlayerSpeed = 4.0f; // Meter/Seconds

	//
	// Handle input
	// 
	for (uint8 controllerIndex = 0; controllerIndex < ArrayCount(input->Controllers); ++controllerIndex)
	{
		GameControllerInput* controller = GetController(input, controllerIndex);


		if (controller->IsAnalog)
		{
			//NOTE: Use analog movement tuning
			if (controller->LeftStickAverageX != 0)
			{
				playerAcceleration.X = controller->LeftStickAverageX;

				if (controller->LeftStickAverageX > 0)
				{
					gameState->PlayerFacingDirection = 0;
				}
				else
				{
					gameState->PlayerFacingDirection = 2;
				}
			}

			if (controller->LeftStickAverageY != 0)
			{
				playerAcceleration.Y = controller->LeftStickAverageY;

				if (controller->LeftStickAverageY > 0)
				{
					gameState->PlayerFacingDirection = 1;
				}
				else
				{
					gameState->PlayerFacingDirection = 3;
				}
			}
		}
		else
		{
			//NOTE: Use digital movement tuning
			if (controller->MoveRight.EndedDown)
			{
				playerAcceleration.X = 1.0f;
				gameState->PlayerFacingDirection = 0;
			}
			if (controller->MoveUp.EndedDown)
			{
				playerAcceleration.Y = 1.0f;
				gameState->PlayerFacingDirection = 1;
			}
			if (controller->MoveDown.EndedDown)
			{
				playerAcceleration.Y = -1.0f;
				gameState->PlayerFacingDirection = 3;
			}
			if (controller->MoveLeft.EndedDown)
			{
				playerAcceleration.X = -1.0f;
				gameState->PlayerFacingDirection = 2;
			}
		}

		if (controller->A.EndedDown)
		{
			gameState->EnableSmoothCamera = !gameState->EnableSmoothCamera;
		}

		if (controller->Start.EndedDown)
		{
			gameMemory->IsInitialized = false;
		}

		if (controller->X.EndedDown)
		{
			PlayerSpeed = 8.0f; // Meter/Seconds
		}
	}


	//
	// Handle Movement after gathering input
	//
	playerAcceleration *= PlayerSpeed;

	// TODO:Use ODE
	// Simulate friction
	real32 friction = -1.0f;
	playerAcceleration += friction * gameState->PlayerVelocity;

	if ((playerAcceleration.X != 0.0f) && (playerAcceleration.Y != 0.0f))
	{
		playerAcceleration.X *= 0.707106781187f;
		playerAcceleration.Y *= 0.707106781187f;
	}

	//
	// Handle collision detection
	//
	MapPosition newPlayerPosition = oldPlayerPosition;

	Vector2d playerDelta = (0.5f * playerAcceleration * (real32)sqaure(input->TimeToAdvance)) + (gameState->PlayerVelocity * (real32)input->TimeToAdvance);

	// Equation of motion
	newPlayerPosition.Offset += playerDelta;

	gameState->PlayerVelocity = playerAcceleration * (real32)input->TimeToAdvance + gameState->PlayerVelocity;

	newPlayerPosition = RecanonicalizePosition(map, newPlayerPosition);

#if 1
	MapPosition playerLeft = newPlayerPosition;
	playerLeft.Offset.X -= 0.5f * playerWidth;
	playerLeft = RecanonicalizePosition(map, playerLeft);

	MapPosition playerRight = newPlayerPosition;
	playerRight.Offset.X += 0.5f * playerWidth;
	playerRight = RecanonicalizePosition(map, playerRight);

	bool32 collided = false;

	MapPosition collidPosition = {};

	if (!IsMapPointEmpty(map, newPlayerPosition))
	{
		collidPosition = newPlayerPosition;
		collided = true;
	}

	if (!IsMapPointEmpty(map, playerLeft))
	{
		collidPosition = playerLeft;
		collided = true;
	}

	if (!IsMapPointEmpty(map, playerRight))
	{
		collidPosition = playerRight;
		collided = true;
	}

	//
	// Update Player Position
	//
	if (collided)
	{
		// We use to reflect if we hit a wall
		Vector2d reflectVector = { };

		if (collidPosition.X < oldPlayerPosition.X)
		{
			reflectVector = Vector2d{ 1, 0 };
		}

		if (collidPosition.X > oldPlayerPosition.X)
		{
			reflectVector = Vector2d{ -1, 0 };
		}

		if (collidPosition.Y < oldPlayerPosition.Y)
		{
			reflectVector = Vector2d{ 0, 1 };
		}

		if (collidPosition.Y > oldPlayerPosition.Y)
		{
			reflectVector = Vector2d{ 0, -1 };
		}

		// If reflectBehaviour is 1, we run with the wall we have collided with, if its 2, we bounce off it 
		uint32 reflectBehaviour = 1;

		gameState->PlayerVelocity = gameState->PlayerVelocity - reflectBehaviour * InnerProduct(gameState->PlayerVelocity, reflectVector) * reflectVector;
	}
	else
	{
		gameState->PlayerPosition = newPlayerPosition;
	}
#else
	// Search in player position for next location after hit detection;
	uint32 minTileX = 0;
	uint32 minTileY = 0;

	uint32 onePastMaxTileX = 0;
	uint32 onePastMaxTileY = 0;

	uint32 tileZ = gameState->PlayerPosition.Z;

	MapPosition bestPoint = gameState->PlayerPosition;
	real32 bestDistanceSq = LengthSq(playerDelta);

	for (uint32 tileY = minTileY; tileY != onePastMaxTileY; tileY++)
	{
		for (uint32 tileX = minTileX; tileX != onePastMaxTileX; tileX++)
		{
			MapPosition testTilePosition = GenerateCeneteredTiledPosition(tileX, tileY, tileZ);
			TileValue tileValue = GetTileValue(map, testTilePosition);

			if (IsTileValueEmpty(tileValue))
			{
				Vector2d minCorner = -0.5f * Vector2d{ map->TileSideInMeters, map->TileSideInMeters };
				Vector2d maxCorner = 0.5f * Vector2d{ map->TileSideInMeters, map->TileSideInMeters };

				MapPositionDifference positionDifference = CalculatePositionDifference(map, &testTilePosition, &newPlayerPosition);
				Vector2d testPosition = ClosestPointInRectangel(minCorner, maxCorner, newPlayerPosition);
				real32 testDistanceSq = LengthSq(testPosition);

				if (testDistanceSq < bestDistanceSq)
				{
					bestPoint = testTilePosition;
					bestDistanceSq = testDistanceSq;
				}
			}
		}
	}
#endif

	if (!AreOnSameTile(oldPlayerPosition, gameState->PlayerPosition))
	{
		TileValue newTileValue = GetTileValue(map, gameState->PlayerPosition);
		if (newTileValue == TileValue::DoorUp)
			gameState->PlayerPosition.Z++;
		else if (newTileValue == TileValue::DoorDown)
			gameState->PlayerPosition.Z--;
	}

	// Smooth scrolling for the camera
	if (gameState->EnableSmoothCamera)
	{
		gameState->CameraPosition.X = gameState->PlayerPosition.X;
		gameState->CameraPosition.Offset.X = gameState->PlayerPosition.Offset.X;
		gameState->CameraPosition.Y = gameState->PlayerPosition.Y;
		gameState->CameraPosition.Offset.Y = gameState->PlayerPosition.Offset.Y;
	}
	else
	{
		MapPositionDifference diff = CalculatePositionDifference(map, &gameState->PlayerPosition, &gameState->CameraPosition);

		if (diff.DXY.X > (9.0f * map->TileSideInMeters))
		{
			gameState->CameraPosition.X += 17;
		}
		else if (diff.DXY.X < -(9.0f * map->TileSideInMeters))
		{
			gameState->CameraPosition.X -= 17;
		}

		if (diff.DXY.Y > (5.0f * map->TileSideInMeters))
		{
			gameState->CameraPosition.Y += 9;
		}
		else if (diff.DXY.Y < -(5.0f * map->TileSideInMeters))
		{
			gameState->CameraPosition.Y -= 9;
		}
	}

	gameState->CameraPosition.Z = gameState->PlayerPosition.Z;

	//
	// Draw New game status
	//
	int32 tileSideInPixels = 60;
	real32 MetersToPixels = (real32)tileSideInPixels / (real32)map->TileSideInMeters;

	Vector2d screenCenter = { 0.5f * (real32)screenBuffer->Width, 0.5f * (real32)screenBuffer->Height };

	DrawBitmap(&gameState->Background, screenBuffer, 0, 0);

	for (int32 relRow = -10; relRow < 10; ++relRow)
	{
		for (int32 relColumn = -20; relColumn < 20; ++relColumn)
		{
			uint32 column = gameState->CameraPosition.X + relColumn;
			uint32 row = gameState->CameraPosition.Y + relRow;
			uint32 floor = gameState->CameraPosition.Z;

			TileValue tileValue = GetTileValue(map, column, row, floor);

			if (tileValue != TileValue::Invalid && tileValue != TileValue::Empty)
			{
				real32 colour = 0.5f;

				if (tileValue == TileValue::Wall)
				{
					colour = 1.0f;
				}

				if (tileValue > TileValue::Wall)
				{
					colour = 0.25f;
				}

				if ((column == gameState->CameraPosition.X) && (row == gameState->CameraPosition.Y))
				{
					colour = 0;
				}

				Vector2d center = { (screenCenter.X - MetersToPixels * gameState->CameraPosition.Offset.X) + ((real32)relColumn) * tileSideInPixels, (screenCenter.Y + MetersToPixels * gameState->CameraPosition.Offset.Y) - ((real32)relRow) * tileSideInPixels };

				Vector2d halfTileSide = { 0.5f * tileSideInPixels , 0.5f * tileSideInPixels };

				Vector2d min = center - halfTileSide;
				Vector2d max = center + halfTileSide;

				DrawRectangle(screenBuffer, min, max, { colour, colour, colour });
			}
		}
	}

	MapPositionDifference camDiff = CalculatePositionDifference(map, &gameState->PlayerPosition, &gameState->CameraPosition);

	real32 playerR = 1.0f;
	real32 playerG = 1.0f;
	real32 playerB = 0.0f;

	real32 playerGroundPointX = screenCenter.X + MetersToPixels * camDiff.DXY.X;
	real32 playerGroundPointY = screenCenter.Y - MetersToPixels * camDiff.DXY.Y;

	Vector2d playerDim = { MetersToPixels * playerWidth, MetersToPixels * playerHeight };
	Vector2d playerLeftTop = { playerGroundPointX - 0.5f * MetersToPixels * playerWidth, playerGroundPointY - MetersToPixels * playerHeight };

	DrawRectangle(screenBuffer, playerLeftTop, playerLeftTop + playerDim, { playerR, playerG, playerB });

	PlayerBitMap* playerFacingDirectionMap = &gameState->playerBitMaps[gameState->PlayerFacingDirection];

	DrawBitmap(&playerFacingDirectionMap->Head, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
	DrawBitmap(&playerFacingDirectionMap->Cape, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
	DrawBitmap(&playerFacingDirectionMap->Torso, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
}

DllExport void GameUpdateAudio(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer* audioBuffer)
{
#if 0
	FillAudioBuffer(thread, gameMemory, audioBuffer);
#endif // 0
}

void DrawRectangle(
	GameScreenBuffer* gameScreenBuffer,
	Vector2d vecMin, Vector2d vecMax,
	Colour colour)
{
	int32 minX = RoundReal32ToInt32(vecMin.X);
	int32 maxX = RoundReal32ToInt32(vecMax.X);

	int32 minY = RoundReal32ToInt32(vecMin.Y);
	int32 maxY = RoundReal32ToInt32(vecMax.Y);

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

LoadedBitmap DebugLoadBmp(ThreadContext* thread, PlatformReadEntireFile* readFile, const char* fileName)
{
	LoadedBitmap result = {};

	DebugFileResult bmp = readFile(thread, fileName);

	if (bmp.FileSize != 0)
	{
		BitmapHeader* header = (BitmapHeader*)bmp.Memory;

		uint32* pixels = (uint32*)((uint8*)bmp.Memory + header->BitmapOffset);

		uint32* source = pixels;

		BitScanResult redShift = FindLeastSigifigantSetBit(header->RedMask);
		BitScanResult greenShift = FindLeastSigifigantSetBit(header->GreeenMask);
		BitScanResult blueShift = FindLeastSigifigantSetBit(header->BlueMask);
		BitScanResult alphaShift = FindLeastSigifigantSetBit(header->AlphaMask);

		Assert(redShift.Found);
		Assert(greenShift.Found);
		Assert(blueShift.Found);
		Assert(alphaShift.Found);

		for (int32 y = 0; y < header->Height; y++)
		{
			for (int32 x = 0; x < header->Width; x++)
			{
				uint32 alpha = (*source >> alphaShift.Index) << 24;
				uint32 red = (*source >> redShift.Index) << 16;
				uint32 green = (*source >> greenShift.Index) << 8;
				uint32 blue = (*source >> blueShift.Index) << 0;

				uint32 r = alpha | red | green | blue;

				*source = r;

				source++;
			}
		}

		result.Data = pixels;
		result.Width = header->Width;
		result.Height = header->Height;
	}

	return result;
}

void DrawBitmap(LoadedBitmap* bitmap, GameScreenBuffer* screenBuffer, real32 realX, real32 realY, int32 alignX, int32 alignY)
{
	realX -= alignX;
	realY -= alignY;

	int64 minX = RoundReal32ToInt32(realX);
	int64 minY = RoundReal32ToInt32(realY);
	int32 maxX = RoundReal32ToInt32(realX + (real32)bitmap->Width);
	int32 maxY = RoundReal32ToInt32(realY + (real32)bitmap->Height);

	// Clip to the nearest valid pixel
	int64 sourceOffsetX = 0;
	if (minX < 0)
	{
		sourceOffsetX = -minX;
		minX = 0;
	}

	int64 sourceOffsetY = 0;

	if (minY < 0)
	{
		sourceOffsetY = -minY - 1;
		minY = 0;
	}

	if (maxX > screenBuffer->Width)
		maxX = screenBuffer->Width;

	if (maxY > screenBuffer->Height)
		maxY = screenBuffer->Height;

	uint32* sourceRow = bitmap->Data + bitmap->Width * bitmap->Height;
	sourceRow += (-sourceOffsetY * bitmap->Width) + sourceOffsetX;

	uint8* destRow = (((uint8*)screenBuffer->Memory) + minX * screenBuffer->BytesPerPixel + minY * screenBuffer->Pitch);

	for (int64 y = minY; y < maxY; y++)
	{
		uint32* dest = (uint32*)destRow;
		uint32* source = sourceRow;

		for (int64 x = minX; x < maxX; x++)
		{
			// Enabling this cause game to crash becasue too much calculation for the cpu on 60fps
#if 1
			// Linear blend
			real32 a = (real32)((*source >> 24) & 0xFF) / 255.0f;
			real32 sr = (real32)((*source >> 16) & 0xFF);
			real32 sg = (real32)((*source >> 8) & 0xFF);
			real32 sb = (real32)((*source >> 0) & 0xFF);

			real32 dr = (real32)((*dest >> 16) & 0xFF);
			real32 dg = (real32)((*dest >> 8) & 0xFF);
			real32 db = (real32)((*dest >> 0) & 0xFF);

			real32 r = (1.0f - a) * dr + a * sr;
			real32 g = (1.0f - a) * dg + a * sg;
			real32 b = (1.0f - a) * db + a * sb;

			uint32 result = (((uint32)(r + 0.5f) << 16) | ((uint32)(g + 0.5f) << 8) | ((uint32)(b + 0.5f)) << 0);

			*dest = result;
#else
			// Alpha Test
			if (*source >> 24 > 124)
			{
				*dest = *source;
			}
#endif // 0


			dest++;
			source++;
		}

		destRow += screenBuffer->Pitch;
		sourceRow -= bitmap->Width;
	}
}

void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer* soundBuffer)
{
	DebugFileResult file = gameMemory->ReadFile(thread, "resources/Water_Splash_SeaLion_Fienup_001.wav");

	if (file.Memory)
	{
		GameAudioBuffer* result = ReadAudioBufferData(file.Memory);

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
