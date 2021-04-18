#include "Leena.h"

// GRAPHICS
LoadedBitmap DebugLoadBmp(ThreadContext* thread, PlatformReadEntireFile* readFile, const char* fileName);
void DrawRectangle(ScreenBuffer* gameScreenBuffer, V2 vecMin, V2 vecMax, Colour colour);
void DrawBitmap(LoadedBitmap* bitmap, ScreenBuffer* screenBuffer, r32 realX, r32 realY, i32 alignX = 0, i32 alignY = 0, r32 cAlpha = 1.0f);

// AUDIO
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, AudioBuffer* soundBuffer);
AudioBuffer* ReadAudioBufferData(void* memory);

void InitializePlayer(GameState* state);
void MoveEntity(GameMemory* gameMemory, Map* map, GameState* gameState, Entity entity, r32 timeToAdvance, V2 playerAcceleration);
u32 AddLowEntity(GameState* gameState, EntityType type);
b32 TestWall(r32& tMin, r32 wall, r32 relX, r32 relY, r32 playerDeltaX, r32 playerDeltaY, r32 minY, r32 maxY);
u32 AddWall(GameState* gameState, MapPosition position);
void SetCamera(GameState* gameState, MapPosition newPosition);

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
		InitilizePool(&gameState->WorldMemoryPool, gameMemory->PermanentStorageSize - sizeof(GameState), (u8*)gameMemory->PermanentStorage + sizeof(GameState));

		gameState->World = PushSize(&gameState->WorldMemoryPool, World);
		World* world = gameState->World;
		world->Map = PushSize(&gameState->WorldMemoryPool, Map);

		Map* map = world->Map;

		initializeMap(&gameState->WorldMemoryPool, map);

		gameState->Background = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_background.bmp");

		PlayerBitMap* playerBitMap;
		playerBitMap = gameState->BitMaps;

		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->AlignX = 72;
		playerBitMap->AlignY = 182;

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->AlignX = 72;
		playerBitMap->AlignY = 182;

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->AlignX = 72;
		playerBitMap->AlignY = 182;

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->AlignX = 72;
		playerBitMap->AlignY = 182;

		AddLowEntity(gameState, EntityType::Null);
		// 0 is reseved as a null entity
		gameState->HighEntitiesCount = 1;

		InitializePlayer(gameState);

		MapPosition cameraPosition = { 17 / 2, 9 / 2, 0 };
		SetCamera(gameState, cameraPosition);

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

		for (u32 screenIndex = 0; screenIndex < 2; ++screenIndex)
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

					if (tileValue == TileValue::Wall)
					{
						AddWall(gameState, GenerateCeneteredTiledPosition(absTileX, absTileY, absTileZ));
					}
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
			player->Low->Speed = 60.0f;
		}
		else
		{
			player->Low->Speed = 30.0f;
		}
		if (keyboard->A.EndedDown && player->High->Z == 0.0f)
		{
			player->High->dZ = 3.0f;
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
			player->Low->Speed = 60.0f;
		}
		else
		{
			player->Low->Speed = 30.0f;
		}
		if (controller->A.EndedDown && player->High->dZ == 0)
		{
			player->High->dZ = 3.0f;
		}
	}

	for (u32 highEntityIndex = 1; highEntityIndex < gameState->HighEntitiesCount; highEntityIndex++)
	{
		Entity entity = {};

		entity.High = gameState->HighEntities + highEntityIndex;
		entity.LowEntityIndex = entity.High->LowEntityIndex;
		entity.Low = gameState->LowEntities + entity.LowEntityIndex;

		if (entity.Low->Type == EntityType::Player)
		{
			MoveEntity(gameMemory, map, gameState, entity, (r32)input->TimeToAdvance, playerAcceleration);
		}
	}

	// Smooth scrolling for the camera
	Entity* playerEntity = &gameState->PlayerEntity;

	MapPosition NewCameraP = gameState->CameraPosition;

	// Smooth camera
	NewCameraP.X = playerEntity->Low->Position.X;
	NewCameraP.Y = playerEntity->Low->Position.Y;
	NewCameraP.Z = playerEntity->Low->Position.Z;
	NewCameraP.Offset = playerEntity->Low->Position.Offset;

	SetCamera(gameState, NewCameraP);

	//
	// Draw New game status
	//
	i32 tileSideInPixels = 60;
	r32 metersToPixels = (r32)tileSideInPixels / (r32)map->TileSideInMeters;

	V2 screenCenter = 0.5f * V2{ (r32)screenBuffer->Width, (r32)screenBuffer->Height };

	DrawBitmap(&gameState->Background, screenBuffer, 0, 0);

	// Render Entities
	for (u8 highEntityIndex = 1; highEntityIndex <= gameState->HighEntitiesCount; highEntityIndex++)
	{
		HighEntity* highEntity = gameState->HighEntities + highEntityIndex;
		LowEntity* lowEntity = gameState->LowEntities + highEntity->LowEntityIndex;

		r32 dt = (r32)input->TimeToAdvance;
		r32 ddZ = -9.8f;
		highEntity->Z = 0.5f * ddZ * Square(dt) + highEntity->dZ * dt + highEntity->Z;
		highEntity->dZ = ddZ * dt + highEntity->dZ;

		if (highEntity->Z < 0)
		{
			highEntity->Z = 0;
		}
		r32 cAlpha = 1.0f - 0.5f * highEntity->Z;
		if (cAlpha < 0)
		{
			cAlpha = 0.0f;
		}

		r32 playerR = 1.0f;
		r32 playerG = 1.0f;
		r32 playerB = 0.0f;

		r32 playerGroundPointX = screenCenter.X + metersToPixels * highEntity->Position.X;
		r32 playerGroundPointY = screenCenter.Y - metersToPixels * highEntity->Position.Y;

		r32 z = -metersToPixels * highEntity->Z;

		if (lowEntity->Type == EntityType::Player)
		{
			PlayerBitMap* playerFacingDirectionMap = &gameState->BitMaps[highEntity->FacingDirection];

			DrawBitmap(&playerFacingDirectionMap->Head, screenBuffer, playerGroundPointX, playerGroundPointY + z, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
			DrawBitmap(&playerFacingDirectionMap->Cape, screenBuffer, playerGroundPointX, playerGroundPointY + z, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
			DrawBitmap(&playerFacingDirectionMap->Torso, screenBuffer, playerGroundPointX, playerGroundPointY + z, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY);
			DrawBitmap(&playerFacingDirectionMap->Shadow, screenBuffer, playerGroundPointX, playerGroundPointY, playerFacingDirectionMap->AlignX, playerFacingDirectionMap->AlignY, cAlpha);
		}
		else
		{
			V2 entityLeftTop = { playerGroundPointX - 0.5f * metersToPixels * lowEntity->Width, playerGroundPointY - 0.5f * metersToPixels * lowEntity->Height };
			V2 entityDim = { metersToPixels * lowEntity->Width, metersToPixels * lowEntity->Height };

			DrawRectangle(screenBuffer, entityLeftTop, entityLeftTop + entityDim, { playerR, playerG, playerB });
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

void DrawBitmap(LoadedBitmap* bitmap, ScreenBuffer* screenBuffer, r32 realX, r32 realY, i32 alignX, i32 alignY, r32 cAlpha)
{
	realX -= alignX;
	realY -= alignY;

	i64 minX = RoundReal32ToInt32(realX);
	i64 minY = RoundReal32ToInt32(realY);
	i64 maxX = minX + bitmap->Width;
	i64 maxY = minY + bitmap->Height;

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

	u32* sourceRow = bitmap->Data + bitmap->Width * (bitmap->Height - 1);
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
			a *= cAlpha;

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

void MoveEntity(GameMemory* gameMemory, Map* map, GameState* gameState, Entity entity, r32 timeToAdvance, V2 playerAcceleration)
{
	// In Meters
	V2 oldPlayerPosition = entity.High->Position;

	r32 playerAccelerationLength = LengthSq(playerAcceleration);

	// Normlazie vetor to make it length of 1
	if (playerAccelerationLength > 1.0f)
	{
		playerAcceleration *= 1 / SqaureRoot(playerAccelerationLength);
	}

	//
	// Handle Movement after gathering input
	//
	playerAcceleration *= entity.Low->Speed;

	// TODO:Use ODE
	// Simulate friction
	r32 friction = -8.0f;

	playerAcceleration += friction * entity.High->Velocity;

	V2 playerDelta = (0.5f * playerAcceleration * Sqaure(timeToAdvance) + (entity.High->Velocity * timeToAdvance));

	// Equation of motion
	entity.High->Velocity = playerAcceleration * timeToAdvance + entity.High->Velocity;

	V2 newPlayerPosition = oldPlayerPosition + playerDelta;

	for (u32 iteration = 0; iteration < 4; iteration++)
	{
		// The relative distance to the closest thing we hit, between 0 and 1
		// 0 didnt move, 1 moved the full amount
		r32 tMin = 1.0f;
		V2 wallNormal = {};
		u32 hitHighEntityIndex = 0;

		V2 desiredPosition = entity.High->Position + playerDelta;

		for (u32 testHighEntityIndex = 1; testHighEntityIndex < gameState->HighEntitiesCount; testHighEntityIndex++)
		{
			if (testHighEntityIndex != entity.Low->HighEntityIndex)
			{
				Entity testEntity = {};
				testEntity.High = gameState->HighEntities + testHighEntityIndex;
				testEntity.LowEntityIndex = testEntity.High->LowEntityIndex;
				testEntity.Low = gameState->LowEntities + testEntity.LowEntityIndex;

				if (testEntity.Low->Collides)
				{
					r32 diameterWidth = testEntity.Low->Width + entity.Low->Width;
					r32 diameterHegiht = testEntity.Low->Height + entity.Low->Height;

					V2 minCorner = -0.5f * V2{ diameterWidth, diameterHegiht };
					V2 maxCorner = 0.5f * V2{ diameterWidth, diameterHegiht };

					V2 relDifference = entity.High->Position - testEntity.High->Position;

					// Side walls

					// Left Wall
					if (TestWall(tMin, minCorner.X, relDifference.X, relDifference.Y, playerDelta.X, playerDelta.Y, minCorner.Y, maxCorner.Y))
					{
						hitHighEntityIndex = testHighEntityIndex;
						wallNormal = V2{ -1, 0 };
					}

					// Right wall
					if (TestWall(tMin, maxCorner.X, relDifference.X, relDifference.Y, playerDelta.X, playerDelta.Y, minCorner.Y, maxCorner.Y))
					{
						hitHighEntityIndex = testHighEntityIndex;
						wallNormal = V2{ 1, 0 };
					}

					// Upper walls
					if (TestWall(tMin, minCorner.Y, relDifference.Y, relDifference.X, playerDelta.Y, playerDelta.X, minCorner.X, maxCorner.X))
					{
						hitHighEntityIndex = testHighEntityIndex;
						wallNormal = V2{ 0, -1 };
					}

					if (TestWall(tMin, maxCorner.Y, relDifference.Y, relDifference.X, playerDelta.Y, playerDelta.X, minCorner.X, maxCorner.X))
					{
						hitHighEntityIndex = testHighEntityIndex;
						wallNormal = V2{ 0, 1 };
					}
				}
			}
		}

		entity.High->Position += tMin * playerDelta;

		if (hitHighEntityIndex)
		{
			entity.High->Velocity = entity.High->Velocity - 1 * InnerProduct(entity.High->Velocity, wallNormal) * wallNormal;
			playerDelta = desiredPosition - entity.High->Position;
			playerDelta = playerDelta - 1 * InnerProduct(playerDelta, wallNormal) * wallNormal;

			HighEntity* hitEntity = gameState->HighEntities + hitHighEntityIndex;
			LowEntity* lowHitEntity = gameState->LowEntities + hitEntity->LowEntityIndex;
			entity.High->PositionZ += lowHitEntity->TileZ;
		}
		else
		{
			break;
		}
	}

	if (entity.High->Velocity.X != 0.0f || entity.High->Velocity.Y != 0.0f)
	{
		if (AbsoluteValue(entity.High->Velocity.X) > AbsoluteValue(entity.High->Velocity.Y))
		{
			if (entity.High->Velocity.X > 0)
			{
				entity.High->FacingDirection = 0;
			}
			else
			{
				entity.High->FacingDirection = 2;
			}
		}
		else if (AbsoluteValue(entity.High->Velocity.X) < AbsoluteValue(entity.High->Velocity.Y))
		{
			if (entity.High->Velocity.Y > 0)
			{
				entity.High->FacingDirection = 1;
			}
			else
			{
				entity.High->FacingDirection = 3;
			}
		}
	}

	entity.Low->Position = MapIntoTileSpace(map, gameState->CameraPosition, entity.High->Position);
}

void InitializePlayer(GameState* state)
{
	u32 playerIndex = AddLowEntity(state, EntityType::Player);
	state->PlayerEntity = { playerIndex , GetLowEntity(state, playerIndex) };

	Entity* entity = &state->PlayerEntity;

	// Order: X Y Z Offset
	entity->Low->Position = { 2, 4, 0, { 0.0f, 0.0f } };

	entity->Low->Collides = true;
	entity->Low->Height = 1.0f;
	entity->Low->Width = 0.75f;
	entity->Low->Speed = 30.0f; // M/S2

	MakeEntityHighFreq(state, playerIndex);

	entity->High = state->HighEntities + entity->Low->HighEntityIndex;
}

u32 AddLowEntity(GameState* gameState, EntityType type)
{
	u32 entityIndex = 0;
	if (gameState->LowEntitiesCount < ArrayCount(gameState->LowEntities))
	{
		entityIndex = gameState->LowEntitiesCount++;
		gameState->LowEntities[entityIndex] = {};
		gameState->LowEntities[entityIndex].Type = type;
	}
	return entityIndex;
}

u32 AddWall(GameState* gameState, MapPosition position)
{
	u32 entityIndex = AddLowEntity(gameState, EntityType::Wall);

	LowEntity* entity = GetLowEntity(gameState, entityIndex);

	// Order: X Y Z Offset
	entity->Position = position;

	entity->Collides = true;
	entity->Height = gameState->World->Map->TileSideInMeters;
	entity->Width = entity->Height;

	return entityIndex;
}

void MakeEntityHighFreq(GameState* gameState, u32 index)
{
	LowEntity* lowEntity = &gameState->LowEntities[index];

	if (!lowEntity->HighEntityIndex)
	{
		if (gameState->HighEntitiesCount < ArrayCount(gameState->HighEntities))
		{
			u32 highEntityIndex = gameState->HighEntitiesCount++;
			HighEntity* highEntity = &gameState->HighEntities[highEntityIndex];

			MapPositionDifference Diff = CalculatePositionDifference(gameState->World->Map, &lowEntity->Position, &gameState->CameraPosition);

			highEntity->Position = Diff.DXY;
			highEntity->Velocity = V2{ 0, 0 };
			highEntity->PositionZ = lowEntity->Position.Z;
			highEntity->FacingDirection = 0;

			highEntity->LowEntityIndex = index;
			lowEntity->HighEntityIndex = highEntityIndex;
		}
		else
		{
			InvalidCodePath;
		}
	}
}

void MakeEntityLowFreq(GameState* gameState, u32 index)
{
	LowEntity* lowEntity = &gameState->LowEntities[index];
	u32 highEntityIndex = lowEntity->HighEntityIndex;

	if (highEntityIndex)
	{
		u32 lastHighEntityIndex = gameState->HighEntitiesCount - 1;

		if (highEntityIndex != gameState->HighEntitiesCount)
		{
			HighEntity* lastHighEntity = gameState->HighEntities + lastHighEntityIndex;
			HighEntity* deletedEntity = gameState->HighEntities + highEntityIndex;
			deletedEntity = lastHighEntity;
			gameState->LowEntities[lastHighEntity->LowEntityIndex].HighEntityIndex = highEntityIndex;
		}

		gameState->HighEntitiesCount--;
		lowEntity->HighEntityIndex = 0;
	}
}

void SetCamera(GameState* gameState, MapPosition newPosition)
{
	Map* map = gameState->World->Map;

	MapPositionDifference dCamera = CalculatePositionDifference(map, &newPosition, &gameState->CameraPosition);
	gameState->CameraPosition = newPosition;

	// Its negative so everything is move against the camera, now with the camera.
	V2 entityOffsetForFrame = -dCamera.DXY;

	// 3 screens in each direction
	r32 tileSpanX = 17.0f * 3.0f;
	r32 tileSpanY = 9.0f * 3.0f;

	R2 cameraBounds = RectCenterHalfDim(V2{ 0, 0 }, map->TileSideInMeters * V2{ tileSpanX, tileSpanY });

	OffsetAndCheckFrequencyByArea(gameState, entityOffsetForFrame, cameraBounds);

	r32 minTileX = newPosition.X - (tileSpanX / 2);
	r32 minTileY = newPosition.Y - (tileSpanY / 2);

	r32 maxTileX = newPosition.X + (tileSpanX / 2);
	r32 maxTileY = newPosition.Y + (tileSpanY / 2);

	for (u32 entityIndex = 1; entityIndex < gameState->LowEntitiesCount; entityIndex++)
	{
		LowEntity* entity = gameState->LowEntities + entityIndex;

		if (!entity->HighEntityIndex)
		{
			if ((entity->Position.Z == newPosition.Z) &&
				(entity->Position.X >= minTileX) &&
				(entity->Position.X <= maxTileX) &&
				(entity->Position.Y >= minTileY) &&
				(entity->Position.Y <= maxTileY))
			{
				MakeEntityHighFreq(gameState, entityIndex);
			}
		}
	}
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
