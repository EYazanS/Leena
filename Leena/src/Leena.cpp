#include "Leena.h"

// GRAPHICS
LoadedBitmap DebugLoadBmp(ThreadContext* thread, PlatformReadEntireFile* readFile, const char* fileName);
void DrawRectangle(ScreenBuffer* gameScreenBuffer, V2 vecMin, V2 vecMax, Colour colour);
void DrawBitmap(LoadedBitmap* bitmap, ScreenBuffer* screenBuffer, r32 realX, r32 realY, i32 alignX = 0, i32 alignY = 0);

// AUDIO
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, AudioBuffer* soundBuffer);
AudioBuffer* ReadAudioBufferData(void* memory);

void RenderEntity(ScreenBuffer* screenBuffer, Map* map, GameState* gameState, V2 screenCenter, r32 metersToPixels, Entity* entity);
void InitializePlayer(GameState* state);
void MoveEntity(GameMemory* gameMemory, Map* map, GameState* gameState, Entity* entity, r32 timeToAdvance, V2 playerAcceleration);
u32 AddEntity(GameState* gameState);

b32 TestWall(r32& tMin, r32 wall, r32 relX, r32 relY, r32 playerDeltaX, r32 playerDeltaY, r32 minY, r32 maxY);

// To pack the struct tightly and prevent combiler from 
// aligning the fields 
#pragma pack(push, 1)
struct BitmapHeader
{
	u16 FileType;
	u32 FileSize;
	u16 Reserved1;
	u16 Reserved2;
	u32 BitmapOffset;
	u32 Size;
	i32 Width;
	i32 Height;
	u16 Planes;
	u16 BitsPerPixel;
	u32 Compression;
	u32 SizeOfBitmap;
	i32 HorzResoloution;
	i32 VertResoloution;
	u32 ColoursUsed;
	u32 ColoursImportant;

	u32 RedMask;
	u32 GreeenMask;
	u32 BlueMask;
	u32 AlphaMask;
};
#pragma pack(pop)

DllExport void GameUpdateAndRender(ThreadContext* thread, GameMemory* gameMemory, ScreenBuffer* screenBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermanentStorage;

	if (!gameMemory->IsInitialized)
	{
		InitializePlayer(gameState);

		gameState->Background = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_background.bmp");

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

		InitilizePool(&gameState->WorldMemoryPool, gameMemory->PermanentStorageSize - sizeof(GameState), (u8*)gameMemory->PermanentStorage + sizeof(GameState));

		gameState->World = PushSize(&gameState->WorldMemoryPool, World);
		World* world = gameState->World;
		world->Map = PushSize(&gameState->WorldMemoryPool, Map);

		Map* map = world->Map;

		initializeMap(&gameState->WorldMemoryPool, map);

		u32 sandomNumberIndex = 0;

		u32 tilesPerWidth = 17;
		u32 tilesPerHeight = 9;
		u32 screenX = 0;
		u32 screenY = 0;
		u32 absTileZ = 0;

		b32 doorLeft = false;
		b32 doorRight = false;
		b32 doorTop = false;
		b32 doorBottom = false;
		b32 doorUp = false;
		b32 doorDown = false;

		for (u32 screenIndex = 0; screenIndex < 100; ++screenIndex)
		{
			Assert(sandomNumberIndex < ArrayCount(randomNumberTable));

			u32 randomChoice;

			if (doorUp || doorDown)
			{
				randomChoice = randomNumberTable[sandomNumberIndex++] % 2;
			}
			else
			{
				randomChoice = randomNumberTable[sandomNumberIndex++] % 3;
			}

			b32 createdZDoor = false;

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

			for (u32 tileY = 0; tileY < tilesPerHeight; ++tileY)
			{
				for (u32 tileX = 0; tileX < tilesPerWidth; ++tileX)
				{
					u32 absTileX = screenX * tilesPerWidth + tileX;
					u32 absTileY = screenY * tilesPerHeight + tileY;

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
	Entity* player = &gameState->PlayerEntity;

	//
	// Handle input
	// 
	KeyboardInput* keyboard = &input->Keyboard;

	V2 playerAcceleration = {};

	if (keyboard->IsConnected)
	{
		playerAcceleration = {};

		//NOTE: Use digital movement tuning
		if (keyboard->MoveRight.EndedDown)
		{
			playerAcceleration.X = 1.0f;
		}
		if (keyboard->MoveUp.EndedDown)
		{
			playerAcceleration.Y = 1.0f;
		}
		if (keyboard->MoveDown.EndedDown)
		{
			playerAcceleration.Y = -1.0f;
		}
		if (keyboard->MoveLeft.EndedDown)
		{
			playerAcceleration.X = -1.0f;
		}
		if (keyboard->Start.EndedDown)
		{
			gameMemory->IsInitialized = false;
		}
		if (keyboard->X.EndedDown)
		{
			player->Speed = 60.0f;
		}
		else
		{
			player->Speed = 30.0f;
		}
	}

	ControllerInput* controller = &input->Controller;

	if (controller->IsConnected)
	{
		playerAcceleration = {};

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

		if (controller->Start.EndedDown)
		{
			gameMemory->IsInitialized = false;
		}
		if (controller->X.EndedDown)
		{
			player->Speed = 60.0f;
		}
		else
		{
			player->Speed = 30.0f;
		}
	}

	MoveEntity(gameMemory, map, gameState, &gameState->PlayerEntity, (r32)input->TimeToAdvance, playerAcceleration);

	// Smooth scrolling for the camera
	Entity* playerEntity = &gameState->PlayerEntity;

	gameState->CameraPosition.X = playerEntity->Position.X;
	gameState->CameraPosition.Offset.X = playerEntity->Position.Offset.X;
	gameState->CameraPosition.Y = playerEntity->Position.Y;
	gameState->CameraPosition.Offset.Y = playerEntity->Position.Offset.Y;
	gameState->CameraPosition.Z = playerEntity->Position.Z;

	//
	// Draw New game status
	//
	i32 tileSideInPixels = 60;
	r32 metersToPixels = (r32)tileSideInPixels / (r32)map->TileSideInMeters;

	V2 screenCenter = 0.5f * V2{ (r32)screenBuffer->Width, (r32)screenBuffer->Height };

	DrawBitmap(&gameState->Background, screenBuffer, 0, 0);

	for (i32 relRow = -10; relRow < 10; ++relRow)
	{
		for (i32 relColumn = -20; relColumn < 20; ++relColumn)
		{
			u32 column = gameState->CameraPosition.X + relColumn;
			u32 row = gameState->CameraPosition.Y + relRow;
			u32 floor = gameState->CameraPosition.Z;

			TileValue tileValue = GetTileValue(map, column, row, floor);

			if (tileValue != TileValue::Invalid && tileValue != TileValue::Empty)
			{
				r32 colour = 0.5f;

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

				V2 center = { (screenCenter.X - metersToPixels * gameState->CameraPosition.Offset.X) + ((r32)relColumn) * tileSideInPixels, (screenCenter.Y + metersToPixels * gameState->CameraPosition.Offset.Y) - ((r32)relRow) * tileSideInPixels };

				V2 halfTileSide = { 0.5f * tileSideInPixels , 0.5f * tileSideInPixels };

				V2 min = center - halfTileSide;
				V2 max = center + halfTileSide;

				DrawRectangle(screenBuffer, min, max, { colour, colour, colour });
			}
		}
	}

	// Render player
	RenderEntity(screenBuffer, map, gameState, screenCenter, metersToPixels, &gameState->PlayerEntity);

	for (u8 entityIndex = 1; entityIndex <= gameState->EntitiesCount; entityIndex++)
	{
		Entity* entity = GetEntity(gameState, entityIndex);

		// TODO: Culling based on camera point of view
		if (entity && entity->Exists && entity->Position.Z == gameState->CameraPosition.Z)
		{
			RenderEntity(screenBuffer, map, gameState, screenCenter, metersToPixels, entity);
		}
	}
}

DllExport void GameUpdateAudio(ThreadContext* thread, GameMemory* gameMemory, AudioBuffer* audioBuffer)
{
#if 0
	FillAudioBuffer(thread, gameMemory, audioBuffer);
#endif // 0
}

void DrawRectangle(
	ScreenBuffer* gameScreenBuffer,
	V2 vecMin, V2 vecMax,
	Colour colour)
{
	i32 minX = RoundReal32ToInt32(vecMin.X);
	i32 maxX = RoundReal32ToInt32(vecMax.X);

	i32 minY = RoundReal32ToInt32(vecMin.Y);
	i32 maxY = RoundReal32ToInt32(vecMax.Y);

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
	u32 finalColour =
		(RoundReal32ToInt32(colour.Red * 255.f) << 16) |
		(RoundReal32ToInt32(colour.Green * 255.f) << 8) |
		(RoundReal32ToInt32(colour.Blue * 255.f) << 0);

	u8* endOfBuffer = ((u8*)gameScreenBuffer->Memory) + ((u64)gameScreenBuffer->Pitch * (u64)gameScreenBuffer->Height);

	u8* row = (((u8*)gameScreenBuffer->Memory) + (i64)minX * gameScreenBuffer->BytesPerPixel + (i64)minY * gameScreenBuffer->Pitch);

	for (i32 y = minY; y < maxY; ++y)
	{
		u32* pixel = (u32*)row;

		for (i32 x = minX; x < maxX; ++x)
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

		u32* pixels = (u32*)((u8*)bmp.Memory + header->BitmapOffset);

		u32* source = pixels;

		BitScanResult alphaScan = FindLeastSigifigantSetBit(header->AlphaMask);
		BitScanResult redScan = FindLeastSigifigantSetBit(header->RedMask);
		BitScanResult greenScan = FindLeastSigifigantSetBit(header->GreeenMask);
		BitScanResult blueScan = FindLeastSigifigantSetBit(header->BlueMask);

		Assert(alphaScan.Found);
		Assert(redScan.Found);
		Assert(greenScan.Found);
		Assert(blueScan.Found);

		i32 alphaShift = 24 - (i32)alphaScan.Index;
		i32 redShift = 16 - (i32)redScan.Index;
		i32 greenShift = 8 - (i32)greenScan.Index;
		i32 blueShift = 0 - (i32)blueScan.Index;

		for (i32 y = 0; y < header->Height; y++)
		{
			for (i32 x = 0; x < header->Width; x++)
			{
				u32 pixel = *source;

				u32 alpha = RotateLeft(pixel & header->AlphaMask, alphaShift);
				u32 red = RotateLeft(pixel & header->RedMask, redShift);
				u32 green = RotateLeft(pixel & header->GreeenMask, greenShift);
				u32 blue = RotateLeft(pixel & header->BlueMask, blueShift);

				u32 combinedColor = alpha | red | green | blue;

				*source++ = combinedColor;
			}
		}

		result.Data = pixels;
		result.Width = header->Width;
		result.Height = header->Height;
	}

	return result;
}

void DrawBitmap(LoadedBitmap* bitmap, ScreenBuffer* screenBuffer, r32 realX, r32 realY, i32 alignX, i32 alignY)
{
	realX -= alignX;
	realY -= alignY;

	i64 minX = RoundReal32ToInt32(realX);
	i64 minY = RoundReal32ToInt32(realY);
	i32 maxX = RoundReal32ToInt32(realX + (r32)bitmap->Width);
	i32 maxY = RoundReal32ToInt32(realY + (r32)bitmap->Height);

	// Clip to the nearest valid pixel
	i64 sourceOffsetX = 0;

	if (minX < 0)
	{
		sourceOffsetX = -minX;
		minX = 0;
	}

	i64 sourceOffsetY = 0;

	if (minY < 0)
	{
		sourceOffsetY = -minY - 1;
		minY = 0;
	}

	if (maxX > screenBuffer->Width)
	{
		maxX = screenBuffer->Width;
	}

	if (maxY > screenBuffer->Height)
	{
		maxY = screenBuffer->Height;
	}

	u32* sourceRow = bitmap->Data + bitmap->Width * bitmap->Height;
	sourceRow += (-sourceOffsetY * bitmap->Width) + sourceOffsetX;

	u8* destRow = (((u8*)screenBuffer->Memory) + minX * screenBuffer->BytesPerPixel + minY * screenBuffer->Pitch);

	for (i64 y = minY; y < maxY; y++)
	{
		u32* dest = (u32*)destRow;
		u32* source = sourceRow;

		for (i64 x = minX; x < maxX; x++)
		{
			// Enabling this cause game to crash becasue too much calculation for the cpu on 60fps
#if 1
			// Linear blend
			r32 a = (r32)((*source >> 24) & 0xFF) / 255.0f;
			r32 sr = (r32)((*source >> 16) & 0xFF);
			r32 sg = (r32)((*source >> 8) & 0xFF);
			r32 sb = (r32)((*source >> 0) & 0xFF);

			r32 dr = (r32)((*dest >> 16) & 0xFF);
			r32 dg = (r32)((*dest >> 8) & 0xFF);
			r32 db = (r32)((*dest >> 0) & 0xFF);

			r32 r = (1.0f - a) * dr + a * sr;
			r32 g = (1.0f - a) * dg + a * sg;
			r32 b = (1.0f - a) * db + a * sb;

			u32 result = (((u32)(r + 0.5f) << 16) | ((u32)(g + 0.5f) << 8) | ((u32)(b + 0.5f)) << 0);

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

void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, AudioBuffer* soundBuffer)
{
	DebugFileResult file = gameMemory->ReadFile(thread, "resources/Water_Splash_SeaLion_Fienup_001.wav");

	if (file.Memory)
	{
		AudioBuffer* result = ReadAudioBufferData(file.Memory);

		*soundBuffer = *result;
	}
}

AudioBuffer* ReadAudioBufferData(void* memory)
{
	u8* byte = (u8*)memory; // Get the firs 4 bytes of the memory

	// Move to position 21
	byte += 20;

	AudioBuffer* result = (AudioBuffer*)byte;

	// Move to Data chunk
	char* characterToRead = ((char*)byte);

	characterToRead += sizeof(AudioBuffer);

	while (*characterToRead != 'd' || *(characterToRead + 1) != 'a' || *(characterToRead + 2) != 't' || *(characterToRead + 3) != 'a')
	{
		characterToRead++;
	}

	characterToRead += 4;

	byte = (u8*)(characterToRead);

	u32 bufferSize = *((u32*)byte);

	byte += 4;

	// Get one third of a sec
	u32 totalBytesNeeded = (u32)(result->SamplesPerSec * 0.5f);

	result->BufferSize = totalBytesNeeded;
	result->BufferData = byte;

	return result;
}

void MoveEntity(GameMemory* gameMemory, Map* map, GameState* gameState, Entity* entity, r32 timeToAdvance, V2 playerAcceleration)
{
	// In Meters
	MapPosition oldPlayerPosition = entity->Position;

	r32 playerAccelerationLength = LengthSq(playerAcceleration);

	// Normlazie vetor to make it length of 1
	if (playerAccelerationLength > 1.0f)
	{
		playerAcceleration *= 1 / SqaureRoot(playerAccelerationLength);
	}

	//
	// Handle Movement after gathering input
	//
	playerAcceleration *= entity->Speed;

	// TODO:Use ODE
	// Simulate friction
	r32 friction = -6.0f;

	playerAcceleration += friction * entity->Velocity;

	V2 playerDelta = (0.5f * playerAcceleration * Sqaure(timeToAdvance) + (entity->Velocity * timeToAdvance));
	V2 totalPlayerDelta = {};

	// Equation of motion
	entity->Velocity = playerAcceleration * timeToAdvance + entity->Velocity;

	MapPosition newPlayerPosition = SetOffset(map, oldPlayerPosition, playerDelta);

	// Search in player position for next location after hit detection;
	u32 minTileX = Minimum(oldPlayerPosition.X, newPlayerPosition.X);
	u32 minTileY = Minimum(oldPlayerPosition.Y, newPlayerPosition.Y);
	u32 maxTileX = Maximum(oldPlayerPosition.X, newPlayerPosition.X);
	u32 maxTileY = Maximum(oldPlayerPosition.Y, newPlayerPosition.Y);

	i32 entityTileWidth = CeilReal32ToInt32(entity->Width / map->TileSideInMeters);
	i32 entityTileHeight = CeilReal32ToInt32(entity->Height / map->TileSideInMeters);

	minTileX -= entityTileWidth;
	minTileY -= entityTileHeight;
	maxTileX += entityTileWidth;
	maxTileY += entityTileHeight;

	u32 tileZ = entity->Position.Z;

	r32 tRemaining = 1.0f;

	for (u32 iteration = 0; (iteration < 4) && (tRemaining > 0.0f); iteration++)
	{
		// The relative distance to the closest thing we hit, between 0 and 1
		// 0 didnt move, 1 moved the full amount
		r32 tMin = 1.0f;
		V2 wallNormal = {};

		for (u32 tileY = minTileY; tileY <= maxTileY; tileY++)
		{
			for (u32 tileX = minTileX; tileX <= maxTileX; tileX++)
			{
				MapPosition testTilePosition = GenerateCeneteredTiledPosition(tileX, tileY, tileZ);
				TileValue tileValue = GetTileValue(map, testTilePosition);

				if (!IsTileValueEmpty(tileValue))
				{
					r32 diameterWidth = map->TileSideInMeters + entity->Width;
					r32 diameterHegiht = map->TileSideInMeters + entity->Height;

					V2 minCorner = -0.5f * V2{ diameterWidth, diameterHegiht };
					V2 maxCorner = 0.5f * V2{ diameterWidth, diameterHegiht };

					MapPositionDifference relOldPositionDifference = CalculatePositionDifference(map, &oldPlayerPosition, &testTilePosition);

					V2 relativeVector = relOldPositionDifference.DXY;

					// Side walls
					if (TestWall(tMin, minCorner.X, relativeVector.X, relativeVector.Y, playerDelta.X, playerDelta.Y, minCorner.Y, maxCorner.Y))
					{
						wallNormal = V2{ -1, 0 };
					}

					if (TestWall(tMin, maxCorner.X, relativeVector.X, relativeVector.Y, playerDelta.X, playerDelta.Y, minCorner.Y, maxCorner.Y))
					{
						wallNormal = V2{ 1, 0 };
					}

					// Upper walls
					if (TestWall(tMin, minCorner.Y, relativeVector.Y, relativeVector.X, playerDelta.Y, playerDelta.X, minCorner.X, maxCorner.X))
					{
						wallNormal = V2{ 0, -1 };
					}

					if (TestWall(tMin, maxCorner.Y, relativeVector.Y, relativeVector.X, playerDelta.Y, playerDelta.X, minCorner.X, maxCorner.X))
					{
						wallNormal = V2{ 0, 1 };
					}
				}
			}
		}

		totalPlayerDelta += tMin * playerDelta;
		entity->Velocity = entity->Velocity - 1 * InnerProduct(entity->Velocity, wallNormal) * wallNormal;
		playerDelta = playerDelta - 1 * InnerProduct(playerDelta, wallNormal) * wallNormal;
		tRemaining -= (tMin * tRemaining);
	}

	entity->Position = SetOffset(map, oldPlayerPosition, totalPlayerDelta);


	if (!AreOnSameTile(oldPlayerPosition, entity->Position))
	{
		TileValue newTileValue = GetTileValue(map, entity->Position);

		if (newTileValue == TileValue::DoorUp)
			entity->Position.Z++;
		else if (newTileValue == TileValue::DoorDown)
			entity->Position.Z--;
	}

	if (entity->Velocity.X != 0.0f || entity->Velocity.Y != 0.0f)
	{
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
}

void InitializePlayer(GameState* state)
{
	Entity* entity = &state->PlayerEntity;

	entity->Exists = true;

	// Order: X Y Z Offset
	entity->Position = { 1, 4, 0, { 0.0f, 0.0f } };

	entity->Height = 1.0f;
	entity->Width = 0.75f;
	entity->Speed = 30.0f; // M/S2
}

void RenderEntity(ScreenBuffer* screenBuffer, Map* map, GameState* gameState, V2 screenCenter, r32 metersToPixels, Entity* entity)
{
	MapPositionDifference camDiff = CalculatePositionDifference(map, &entity->Position, &gameState->CameraPosition);

	r32 playerR = 1.0f;
	r32 playerG = 1.0f;
	r32 playerB = 0.0f;

	r32 playerGroundPointX = screenCenter.X + metersToPixels * camDiff.DXY.X;
	r32 playerGroundPointY = screenCenter.Y - metersToPixels * camDiff.DXY.Y;

	V2 playerDim = { metersToPixels * entity->Width, metersToPixels * entity->Height };
	V2 playerLeftTop = { playerGroundPointX - 0.5f * metersToPixels * entity->Width, playerGroundPointY - 0.5f * metersToPixels * entity->Height };

	DrawRectangle(screenBuffer, playerLeftTop, playerLeftTop + playerDim, { playerR, playerG, playerB });

	PlayerBitMap* playerFacingDirectionMap = &gameState->BitMaps[entity->FacingDirection];

	DrawBitmap(&playerFacingDirectionMap->Head, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
	DrawBitmap(&playerFacingDirectionMap->Cape, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
	DrawBitmap(&playerFacingDirectionMap->Torso, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
}

u32 AddEntity(GameState* gameState)
{
	u32 entityIndex = gameState->EntitiesCount++;
	Assert(gameState->EntitiesCount < ArrayCount(gameState->Entities));
	Entity* entity = &gameState->Entities[entityIndex];
	*entity = {};
	return entityIndex;
}

b32 TestWall(r32& tMin, r32 wall, r32 relX, r32 relY, r32 playerDeltaX, r32 playerDeltaY, r32 minY, r32 maxY)
{
	b32 hitWall = false;
	r32 tEpslion = 0.0001f;

	if (playerDeltaX != 0)
	{
		r32 tResult = (wall - relX) / playerDeltaX;

		r32 y = relY + tResult * playerDeltaY;

		if ((tResult >= 0.0f) && (tMin > tResult))
		{
			if ((y >= minY) && (y <= maxY))
			{
				tMin = Maximum(0.0f, tResult - tEpslion);
				hitWall = true;
			}
		}
	}

	return hitWall;
}