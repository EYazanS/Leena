#include "Leena.h"

// GRAPHICS
LoadedBitmap DebugLoadBmp(ThreadContext* thread, PlatformReadEntireFile* readFile, const char* fileName);
void DrawRectangle(GameScreenBuffer* gameScreenBuffer, Vector2d vecMin, Vector2d vecMax, Colour colour);
void DrawBitmap(LoadedBitmap* bitmap, GameScreenBuffer* screenBuffer, real32 realX, real32 realY, int32 alignX = 0, int32 alignY = 0);

// AUDIO
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer* soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);

void RenderEntity(GameScreenBuffer* screenBuffer, Map* map, GameState* gameState, Vector2d screenCenter, real32 metersToPixels, Entity* entity);
void InitializePlayer(GameState* state, uint32  index);
void MoveEntity(GameMemory* gameMemory, Map* map, GameState* gameState, GameControllerInput* controller, Entity* entity, real32 timeToAdvance, Vector2d playerAcceleration);
uint32 AddEntity(GameState* gameState);

void TestWall(real32& tMin, real32 wall, real32 relX, real32 relY, real32 playerDeltaX, real32 playerDeltaY, real32 minY, real32 maxY);

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
		// Reserve the first entity as null entity
		AddEntity(gameState);

		gameState->Background = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_background.bmp");
		gameState->EntityIndexTheCameraIsFollowing = 1;
		PlayerBitMap* playerBitMap;
		playerBitMap = gameState->BitMaps;

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

	//
	// Handle input
	// 
	for (uint8 controllerIndex = 0; controllerIndex < ArrayCount(input->Controllers); controllerIndex++)
	{
		GameControllerInput* controller = GetController(input, controllerIndex);

		if (!controller->IsConnected)
		{
			continue;
		}

		Entity* controlledEntity = GetEntity(gameState, gameState->PlayersIndexesForControllers[controllerIndex]);

		Vector2d playerAcceleration = {};

		if (controlledEntity)
		{
			if (controller->IsAnalog)
			{
				//NOTE: Use analog movement tuning
				playerAcceleration = { controller->LeftStickAverageX , controller->LeftStickAverageY };
			}
			else
			{
				//NOTE: Use digital movement tuning
				if (controller->MoveRight.EndedDown)
				{
					playerAcceleration.X = 1.0f;
				}
				if (controller->MoveUp.EndedDown)
				{
					playerAcceleration.Y = 1.0f;
				}
				if (controller->MoveDown.EndedDown)
				{
					playerAcceleration.Y = -1.0f;
				}
				if (controller->MoveLeft.EndedDown)
				{
					playerAcceleration.X = -1.0f;
				}
			}
		}
		else
		{
			if (controller->Start.EndedDown)
			{
				uint32 controlledEntityIndex = AddEntity(gameState);
				controlledEntity = GetEntity(gameState, controlledEntityIndex);
				InitializePlayer(gameState, controlledEntityIndex);
				gameState->PlayersIndexesForControllers[controllerIndex] = controlledEntityIndex;
			}
		}

		if (controlledEntity && controlledEntity->Exists)
		{
			MoveEntity(gameMemory, map, gameState, controller, controlledEntity, (real32)input->TimeToAdvance, playerAcceleration);
		}
	}

	// Smooth scrolling for the camera
	Entity* cameraFollowingEntity = GetEntity(gameState, gameState->EntityIndexTheCameraIsFollowing);

	if (cameraFollowingEntity)
	{
		if (gameState->EnableSmoothCamera)
		{
			gameState->CameraPosition.X = cameraFollowingEntity->Position.X;
			gameState->CameraPosition.Offset.X = cameraFollowingEntity->Position.Offset.X;
			gameState->CameraPosition.Y = cameraFollowingEntity->Position.Y;
			gameState->CameraPosition.Offset.Y = cameraFollowingEntity->Position.Offset.Y;
		}
		else
		{
			MapPositionDifference diff = CalculatePositionDifference(map, &cameraFollowingEntity->Position, &gameState->CameraPosition);

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

		gameState->CameraPosition.Z = cameraFollowingEntity->Position.Z;
	}
	//
	// Draw New game status
	//
	int32 tileSideInPixels = 60;
	real32 metersToPixels = (real32)tileSideInPixels / (real32)map->TileSideInMeters;

	// TOOD: Check this
	Vector2d screenCenter = 0.5f * Vector2d{ (real32)screenBuffer->Width, (real32)screenBuffer->Height };

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

				Vector2d center = { (screenCenter.X - metersToPixels * gameState->CameraPosition.Offset.X) + ((real32)relColumn) * tileSideInPixels, (screenCenter.Y + metersToPixels * gameState->CameraPosition.Offset.Y) - ((real32)relRow) * tileSideInPixels };

				Vector2d halfTileSide = { 0.5f * tileSideInPixels , 0.5f * tileSideInPixels };

				Vector2d min = center - halfTileSide;
				Vector2d max = center + halfTileSide;

				DrawRectangle(screenBuffer, min, max, { colour, colour, colour });
			}
		}
	}

	for (uint8 entityIndex = 1; entityIndex <= gameState->EntitiesCount; entityIndex++)
	{
		Entity* entity = GetEntity(gameState, entityIndex);

		// TODO: Culling based on camera point of view
		if (entity && entity->Exists && entity->Position.Z == gameState->CameraPosition.Z)
		{
			RenderEntity(screenBuffer, map, gameState, screenCenter, metersToPixels, entity);
		}
	}
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

		BitScanResult alphaScan = FindLeastSigifigantSetBit(header->AlphaMask);
		BitScanResult redScan = FindLeastSigifigantSetBit(header->RedMask);
		BitScanResult greenScan = FindLeastSigifigantSetBit(header->GreeenMask);
		BitScanResult blueScan = FindLeastSigifigantSetBit(header->BlueMask);

		Assert(alphaScan.Found);
		Assert(redScan.Found);
		Assert(greenScan.Found);
		Assert(blueScan.Found);

		int32 alphaShift = 24 - (int32)alphaScan.Index;
		int32 redShift = 16 - (int32)redScan.Index;
		int32 greenShift = 8 - (int32)greenScan.Index;
		int32 blueShift = 0 - (int32)blueScan.Index;

		for (int32 y = 0; y < header->Height; y++)
		{
			for (int32 x = 0; x < header->Width; x++)
			{
				uint32 pixel = *source;

				uint32 alpha = RotateLeft(pixel & header->AlphaMask, alphaShift);
				uint32 red = RotateLeft(pixel & header->RedMask, redShift);
				uint32 green = RotateLeft(pixel & header->GreeenMask, greenShift);
				uint32 blue = RotateLeft(pixel & header->BlueMask, blueShift);

				uint32 combinedColor = alpha | red | green | blue;

				*source++ = combinedColor;
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


void MoveEntity(GameMemory* gameMemory, Map* map, GameState* gameState, GameControllerInput* controller, Entity* entity, real32 timeToAdvance, Vector2d playerAcceleration)
{
	// In Meters
	MapPosition oldPlayerPosition = entity->Position;
	real32 PlayerSpeed = 30.0f; // Meter/Seconds

	real32 playerAccelerationLength = LengthSq(playerAcceleration);

	// Normlazie vetor to make it length of 1
	if (playerAccelerationLength > 1.0f)
	{
		playerAcceleration *= 1 / SqaureRoot(playerAccelerationLength);
	}

	if (controller->A.EndedDown)
	{
		gameState->EnableSmoothCamera = !gameState->EnableSmoothCamera;
	}

	if (controller->Back.EndedDown)
	{
		gameMemory->IsInitialized = false;
	}

	if (controller->X.EndedDown)
	{
		PlayerSpeed = 60.0f; // Meter/Seconds
	}

	//
	// Handle Movement after gathering input
	//
	playerAcceleration *= PlayerSpeed;

	// TODO:Use ODE
	// Simulate friction
	real32 friction = -6.0f;

	playerAcceleration += friction * entity->Velocity;

	Vector2d playerDelta = (0.5f * playerAcceleration * Sqaure(timeToAdvance) + (entity->Velocity * timeToAdvance));

	// Equation of motion
	entity->Velocity = playerAcceleration * timeToAdvance + entity->Velocity;

	MapPosition newPlayerPosition = SetOffset(map, oldPlayerPosition, playerDelta);

#if 0
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

		entity->Velocity = entity->Velocity - reflectBehaviour * InnerProduct(entity->Velocity, reflectVector) * reflectVector;
	}
	else
	{
		entity->Position = newPlayerPosition;
	}
#else
	// Search in player position for next location after hit detection;
#if 0
	uint32 minTileX = Minimum(oldPlayerPosition.X, newPlayerPosition.X);
	uint32 minTileY = Minimum(oldPlayerPosition.Y, newPlayerPosition.Y);
	uint32 onePastMaxTileX = Maximum(oldPlayerPosition.X, newPlayerPosition.X) + 1;
	uint32 onePastMaxTileY = Maximum(oldPlayerPosition.Y, newPlayerPosition.Y) + 1;
#else
	uint32 startTileX = oldPlayerPosition.X;
	uint32 startTileY = oldPlayerPosition.Y;
	uint32 endTileX = newPlayerPosition.X;
	uint32 endTileY = newPlayerPosition.Y;

	int32 deltaX = SingOf(endTileX - startTileX);
	int32 deltaY = SingOf(endTileY - startTileY);
#endif
	uint32 tileZ = entity->Position.Z;
	// The relative distance to the closest thing we hit, between 0 and 1
	// 0 didnt move, 1 moved the full amount
	real32 tMin = 1.0f;

	uint32 tileY = startTileY;
	for (;;)
	{
		uint32 tileX = startTileX;

		for (;;)
		{
			MapPosition testTilePosition = GenerateCeneteredTiledPosition(tileX, tileY, tileZ);
			TileValue tileValue = GetTileValue(map, testTilePosition);

			if (!IsTileValueEmpty(tileValue))
			{
				Vector2d minCorner = -0.5f * Vector2d{ map->TileSideInMeters, map->TileSideInMeters };
				Vector2d maxCorner = 0.5f * Vector2d{ map->TileSideInMeters, map->TileSideInMeters };

				MapPositionDifference relOldPositionDifference = CalculatePositionDifference(map, &oldPlayerPosition, &testTilePosition);

				Vector2d relativeVector = relOldPositionDifference.DXY;

				// Side walls
				TestWall(tMin, minCorner.X, relativeVector.X, relativeVector.Y, playerDelta.X, playerDelta.Y, minCorner.Y, maxCorner.Y);
				TestWall(tMin, maxCorner.X, relativeVector.X, relativeVector.Y, playerDelta.X, playerDelta.Y, minCorner.Y, maxCorner.Y);

				// Upper walls
				TestWall(tMin, minCorner.Y, relativeVector.Y, relativeVector.X, playerDelta.Y, playerDelta.X, minCorner.X, maxCorner.X);
				TestWall(tMin, maxCorner.Y, relativeVector.Y, relativeVector.X, playerDelta.Y, playerDelta.X, minCorner.X, maxCorner.X);
			}

			if (tileX == endTileX)
			{
				break;
			}
			else
			{
				tileX += deltaX;
			}
		}

		if (tileY == endTileY)
		{
			break;
		}
		else
		{
			tileY += deltaY;
		}
	}

	newPlayerPosition = oldPlayerPosition;
	entity->Position = SetOffset(map, newPlayerPosition, (0.9f * tMin) * playerDelta);

#endif

	if (!AreOnSameTile(oldPlayerPosition, entity->Position))
	{
		TileValue newTileValue = GetTileValue(map, entity->Position);

		if (newTileValue == TileValue::DoorUp)
			entity->Position.Z++;
		else if (newTileValue == TileValue::DoorDown)
			entity->Position.Z--;
	}

	if (entity->Velocity.X != 0.0f || entity->Velocity.Y != 0.0f)
		if (AbsoluteValue(entity->Velocity.X) > AbsoluteValue(entity->Velocity.Y))
		{
			if (entity->Velocity.X > 0)
			{
				entity->FacingDirection = 0;
			}
			else
			{
				entity->FacingDirection = 2;
			}
		}
		else if (AbsoluteValue(entity->Velocity.X) < AbsoluteValue(entity->Velocity.Y))
		{
			if (entity->Velocity.Y > 0)
			{
				entity->FacingDirection = 1;
			}
			else
			{
				entity->FacingDirection = 3;
			}
		}
}

void InitializePlayer(GameState* state, uint32  index)
{
	Entity* entity = GetEntity(state, index);

	entity->Exists = true;

	// Order: X Y Z Offset
	entity->Position = { 1, 4, 0, { 0.0f, 0.0f } };

	entity->Height = 1.4f;
	entity->Width = 0.75f * entity->Height;

	if (!GetEntity(state, !state->EntityIndexTheCameraIsFollowing))
	{
		state->EntityIndexTheCameraIsFollowing = index;
	}
}

void RenderEntity(GameScreenBuffer* screenBuffer, Map* map, GameState* gameState, Vector2d screenCenter, real32 metersToPixels, Entity* entity)
{
	MapPositionDifference camDiff = CalculatePositionDifference(map, &entity->Position, &gameState->CameraPosition);

	real32 playerR = 1.0f;
	real32 playerG = 1.0f;
	real32 playerB = 0.0f;

	real32 playerGroundPointX = screenCenter.X + metersToPixels * camDiff.DXY.X;
	real32 playerGroundPointY = screenCenter.Y - metersToPixels * camDiff.DXY.Y;

	Vector2d playerDim = { metersToPixels * entity->Width, metersToPixels * entity->Height };
	Vector2d playerLeftTop = { playerGroundPointX - 0.5f * metersToPixels * entity->Width, playerGroundPointY - metersToPixels * entity->Height };

	DrawRectangle(screenBuffer, playerLeftTop, playerLeftTop + playerDim, { playerR, playerG, playerB });

	PlayerBitMap* playerFacingDirectionMap = &gameState->BitMaps[entity->FacingDirection];

	DrawBitmap(&playerFacingDirectionMap->Head, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
	DrawBitmap(&playerFacingDirectionMap->Cape, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
	DrawBitmap(&playerFacingDirectionMap->Torso, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
}

uint32 AddEntity(GameState* gameState)
{
	uint32 entityIndex = gameState->EntitiesCount++;
	Assert(gameState->EntitiesCount < ArrayCount(gameState->Entities));
	Entity* entity = &gameState->Entities[entityIndex];
	*entity = {};
	return entityIndex;
}

void TestWall(real32& tMin, real32 wall, real32 relX, real32 relY, real32 playerDeltaX, real32 playerDeltaY, real32 minY, real32 maxY)
{
	real32 tEpslion = 0.00001f;

	if (playerDeltaX != 0)
	{
		real32 tResult = (wall - relX) / playerDeltaX;

		real32 y = relY + tResult * playerDeltaY;

		if ((tResult >= 0.0f) && (tMin > tResult))
		{
			if ((y >= minY) && (y <= maxY))
			{
				tMin = Maximum(0.0f, tResult - tEpslion);
			}
		}
	}
}