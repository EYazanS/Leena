#include "Leena.h"

// GRAPHICS
LoadedBitmap DebugLoadBmp(ThreadContext* thread, PlatformReadEntireFile* readFile, const char* fileName);
void DrawRectangle(ScreenBuffer* gameScreenBuffer, V2 vecMin, V2 vecMax, V4 colour);
void DrawBitmap(ScreenBuffer* screenBuffer, LoadedBitmap* bitmap, r32 realX, r32 realY, r32 cAlpha = 1.0f);

// AUDIO
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, AudioBuffer* soundBuffer);
AudioBuffer* ReadAudioBufferData(void* memory);

AddLowEntityResult AddLowEntity(GameState* gameState, EntityType type, WorldPosition* post);
AddLowEntityResult AddWall(GameState* gameState, i32 x, i32 y, i32 z);
AddLowEntityResult AddMonster(GameState* state, WorldPosition* position);
AddLowEntityResult AddFamiliar(GameState* state, WorldPosition* position);

b32 TestWall(r32 WallX, r32 RelX, r32 RelY, r32 PlayerDeltaX, r32 PlayerDeltaY, r32* tMin, r32 MinY, r32 MaxY);

void InitializePlayer(GameState* state);
void MoveEntity(GameState* gameState, Entity entity, r32 timeToAdvance, V2 playerAcceleration);
void SetCamera(GameState* gameState, WorldPosition newPosition);

void UpdateFamiliar(GameState* state, Entity entity, r32 dt);
void UpdateMonster(GameState* state, Entity entity, r32 dt);
inline void PushBitmap(EntityVisiblePieceGroup* group, LoadedBitmap* bitmap, V2 offset, r32 offsetZ, V2 align, r32 alpha = 1, r32 entityZC = 1);
inline void PushRect(EntityVisiblePieceGroup* group, V2 offset, r32 offsetZ, V2 dim, V4 colour, r32 zCoefficient = 1);

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
		initializePool(&gameState->WorldMemoryPool, gameMemory->PermanentStorageSize - sizeof(GameState), (u8*)gameMemory->PermanentStorage + sizeof(GameState));

		gameState->World = PushStruct(&gameState->WorldMemoryPool, World);

		World* world = gameState->World;

		initializeWorld(world);

		gameState->Background = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_background.bmp");
		gameState->Tree = DebugLoadBmp(thread, gameMemory->ReadFile, "test2/tree00.bmp");
		gameState->Rock = DebugLoadBmp(thread, gameMemory->ReadFile, "test2/rock00.bmp");

		PlayerBitMap* playerBitMap;
		playerBitMap = gameState->BitMaps;

		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->Align = { 72, 182 };

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->Align = { 72, 182 };

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->Align = { 72, 182 };

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->Align = { 72, 182 };

		AddLowEntity(gameState, EntityType::Null, {});
		// 0 is reseved as a null entity
		gameState->HighEntitiesCount = 1;

		u32 ScreenX = 0;
		u32 ScreenY = 0;
		u32 ScreenZ = 0;
		u32 TilesPerWidth = 17;
		u32 TilesPerHeight = 9;
		WorldPosition cameraPosition = {};
		u32 CameraTileX = ScreenX * TilesPerWidth + 17 / 2;
		u32 CameraTileY = ScreenY * TilesPerHeight + 9 / 2;
		u32 CameraTileZ = ScreenZ;

		cameraPosition = GetChunkPositionFromWorldPosition(world,
			CameraTileX,
			CameraTileY,
			CameraTileZ);

		SetCamera(gameState, cameraPosition);

		InitializePlayer(gameState);

		u32 randomNumberIndex = 0;

		u32 tilesPerWidth = 17;
		u32 tilesPerHeight = 9;
		u32 screenX = 0;
		u32 screenY = 0;
		u32 absTileZ = 0;

		b32 doorLeft = false;
		b32 doorRight = false;
		b32 doorTop = false;
		b32 doorBottom = false;
		//b32 doorUp = false;
		//b32 doorDown = false;

		for (u32 screenIndex = 0; screenIndex < 50; ++screenIndex)
		{
			Assert(randomNumberIndex < ArrayCount(randomNumberTable));

			u32 randomChoice;

			randomChoice = randomNumberTable[randomNumberIndex++] % 2;
			//if (doorUp || doorDown)
			//{
			//}
			//else
			//{
			//	randomChoice = randomNumberTable[sandomNumberIndex++] % 3;
			//}

			b32 createdZDoor = false;

			/*if (randomChoice == 2)
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
			else*/ if (randomChoice == 1)
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

					u32 tileValue = 1;

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

					//if (tileX == 10 && tileY == 6)
					//{
					//	if (doorUp)
					//	{
					//		tileValue = 2;
					//	}

					//	if (doorDown)
					//	{
					//		tileValue = 2;
					//	}
					//}

					if (tileValue == 2)
					{
						AddWall(gameState, absTileX, absTileY, absTileZ);
					}
				}
			}

			doorLeft = doorRight;

			doorBottom = doorTop;

			/*if (createdZDoor)
			{
				doorDown = !doorDown;
				doorUp = !doorUp;
			}
			else
			{
				doorUp = false;
				doorDown = false;
			}*/

			doorRight = false;
			doorTop = false;

			/*if (randomChoice == 2)
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
			else */if (randomChoice == 1)
			{
				screenX++;
			}
			else
			{
				screenY++;
			}
		}

		WorldPosition monsterPost = GetChunkPositionFromWorldPosition(world, CameraTileX + 2, CameraTileY + 2, CameraTileZ);

		AddMonster(gameState, &monsterPost);

		for (int FamiliarIndex = 0; FamiliarIndex < 5; ++FamiliarIndex)
		{
			i32 FamiliarOffsetX = (randomNumberTable[randomNumberIndex++] % 10) - 7;
			i32 FamiliarOffsetY = (randomNumberTable[randomNumberIndex++] % 10) - 3;

			if ((FamiliarOffsetX != 0) || (FamiliarOffsetY != 0))
			{
				WorldPosition familiarPosition = GetChunkPositionFromWorldPosition(world, CameraTileX + FamiliarOffsetX, CameraTileY + FamiliarOffsetY, CameraTileZ);

				AddFamiliar(gameState, &familiarPosition);
			}
		}

		i32 tileSideInPixels = 60;

		gameState->MetersToPixels = (r32)tileSideInPixels / (r32)world->TileSideInMeters;

		gameMemory->IsInitialized = true;
	}

	World* world = gameState->World;
	Entity* player = &gameState->PlayerEntity;

	//
	// Handle input
	// 
	KeyboardInput* keyboard = &input->Keyboard;

	V2 playerAcceleration = {};

	if (keyboard->IsConnected)
	{
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
			// gameMemory->IsInitialized = false;
		}
	}

	ControllerInput* controller = &input->Controller;

	if (controller->IsConnected)
	{
		if (controller->IsAnalog)
		{
			if (controller->LeftStickAverageX || controller->LeftStickAverageY)
			{
				//NOTE: Use analog movement tuning
				playerAcceleration = { controller->LeftStickAverageX , controller->LeftStickAverageY };
			}
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

	if (keyboard->X.EndedDown || controller->X.EndedDown)
	{
		player->Low->Speed = 60.0f;
	}
	else
	{
		player->Low->Speed = 30.0f;
	}

	if ((keyboard->A.EndedDown || controller->A.EndedDown) && player->High->Z == 0.0f)
	{
		player->High->dZ = 3.0f;
	}

	if (controller->Start.EndedDown)
	{
		gameMemory->IsInitialized = false;
	}
	if (keyboard->X.EndedDown || controller->X.EndedDown)
	{
		player->Low->Speed = 60.0f;
	}
	else
	{
		player->Low->Speed = 30.0f;
	}

	for (u32 highEntityIndex = 1; highEntityIndex < gameState->HighEntitiesCount; highEntityIndex++)
	{
		Entity entity = {};

		entity.High = gameState->HighEntities + highEntityIndex;
		entity.LowEntityIndex = entity.High->LowEntityIndex;
		entity.Low = gameState->LowEntities + entity.LowEntityIndex;

		if (entity.Low->Type == EntityType::Player)
		{
			MoveEntity(gameState, entity, (r32)input->TimeToAdvance, playerAcceleration);
		}
	}

	// Smooth scrolling for the camera
	WorldPosition NewCameraP = gameState->PlayerEntity.Low->Position;

	SetCamera(gameState, NewCameraP);

	//
	// Draw New game status
	//

	r32 metersToPixels = gameState->MetersToPixels;

	V2 screenCenter = 0.5f * V2{ (r32)screenBuffer->Width, (r32)screenBuffer->Height };

#if 1
	DrawRectangle(screenBuffer, {}, { (r32)screenBuffer->Width, (r32)screenBuffer->Height }, { 0.5f,0.5f, 0.5f });
#else
	DrawBitmap(&gameState->Background, screenBuffer, 0, 0);
#endif // 0

	EntityVisiblePieceGroup pieceGroup = {};

	pieceGroup.GameState = gameState;

	// Render Entities
	for (u32 highEntityIndex = 1; highEntityIndex < gameState->HighEntitiesCount; highEntityIndex++)
	{
		pieceGroup.PieceCount = 0;

		HighEntity* highEntity = gameState->HighEntities + highEntityIndex;
		LowEntity* lowEntity = gameState->LowEntities + highEntity->LowEntityIndex;

		r32 dt = (r32)input->TimeToAdvance;

		// TODO: fix
		r32 shadowAlpha = 1.0f - 0.5f * highEntity->Z;

		if (shadowAlpha < 0)
		{
			shadowAlpha = 0.0f;
		}

		Entity entity;

		entity.Low = lowEntity;
		entity.High = highEntity;
		entity.LowEntityIndex = highEntity->LowEntityIndex;

		PlayerBitMap* playerFacingDirectionMap = &gameState->BitMaps[highEntity->FacingDirection];

		switch (lowEntity->Type)
		{
		case  EntityType::Player:
		{
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Head, V2{ 0, 0 }, 0, playerFacingDirectionMap->Align);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Cape, V2{ 0, 0 }, 0, playerFacingDirectionMap->Align);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Torso, V2{ 0, 0 }, 0, playerFacingDirectionMap->Align);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Shadow, V2{ 0, 0 }, 0, playerFacingDirectionMap->Align, shadowAlpha, 0);

			if (lowEntity->MaxHp > 0)
			{
				V2 healthDim = { 0.2f, 0.2f };
				r32 spacing = healthDim.X * 1.5f;

				V2 hpPosition = { -0.5f * (lowEntity->MaxHp - 1) * spacing, -0.40f };
				V2 dHitP = { spacing, 0.0f };

				for (u8 healthIndex = 0; healthIndex < lowEntity->MaxHp; healthIndex++)
				{
					Hitpoint* hitPoint = lowEntity->Hitpoints + healthIndex;

					V4 colour = { 1.0f, 0.0f,  0.0f, 1.0f };

					if (hitPoint->Current == 0)
					{
						colour.R = 0.2f;
						colour.G = 0.2f;
						colour.B = 0.2f;
					}

					PushRect(&pieceGroup, hpPosition, 0, healthDim, colour, 0.0f);

					hpPosition += dHitP;
				}
			}
		} break;

		case EntityType::Wall:
		{
			PushBitmap(&pieceGroup, &gameState->Tree, V2{ 0,0 }, 0, V2{ 40, 80 });
		} break;

		case EntityType::Familiar:
		{
			UpdateFamiliar(gameState, entity, dt);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Head, V2{ 0, 0 }, 0, playerFacingDirectionMap->Align);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Shadow, V2{ 0,0 }, 0, playerFacingDirectionMap->Align, shadowAlpha, 0);

		} break;

		case EntityType::Monster:
		{
			UpdateMonster(gameState, entity, dt);
			PushBitmap(&pieceGroup, &gameState->Rock, V2{ 0,0 }, 0, V2{ 40, 80 });
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Shadow, V2{ 0,0 }, 0, playerFacingDirectionMap->Align, shadowAlpha);

		} break;

		default:
		{
			InvalidCodePath;
		} break;
		}

		// Gravity
		r32 ddZ = -9.8f;

		highEntity->Z = 0.5f * ddZ * Square(dt) + highEntity->dZ * dt + highEntity->Z;

		highEntity->dZ = ddZ * dt + highEntity->dZ;

		if (highEntity->Z < 0)
		{
			highEntity->Z = 0;
		}

		r32 groundPointX = screenCenter.X + metersToPixels * highEntity->Position.X;
		r32 groundPointY = screenCenter.Y - metersToPixels * highEntity->Position.Y;
		r32 entityZ = -metersToPixels * highEntity->Z;

#if 0
		V2 playerLeftTop = { playerGroundPointX - 0.5f * metersToPixels * lowEntity->Width, playerGroundPointY - 0.5f * metersToPixels * lowEntity->Height };
		V2 entityWidthHeight = { lowEntity->Width, lowEntity->Height };
		DrawRectangle(screenBuffer, playerLeftTop, playerLeftTop + metersToPixels * entityWidthHeight, { 1.0f, 1.0f, 0.0f });
#endif

		for (u32 pieceIndex = 0; pieceIndex < pieceGroup.PieceCount; pieceIndex++)
		{
			EntityVisiblePiece* piece = pieceGroup.Pieces + pieceIndex;

			V2 center = { groundPointX + piece->Offset.X, groundPointY + piece->Offset.Y + piece->Z + piece->ZCoefficient * entityZ };

			if (piece->Bitmap)
			{
				DrawBitmap(screenBuffer, piece->Bitmap, center.X, center.Y, piece->Colour.A);
			}
			else
			{
				V2 dim = 0.5f * metersToPixels * piece->Dimensions;
				DrawRectangle(screenBuffer, center - dim, center + dim, piece->Colour);
			}
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
	V4 colour)
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
		(RoundReal32ToInt32(colour.R * 255.f) << 16) |
		(RoundReal32ToInt32(colour.G * 255.f) << 8) |
		(RoundReal32ToInt32(colour.B * 255.f) << 0);

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

void DrawBitmap(ScreenBuffer* screenBuffer, LoadedBitmap* bitmap, r32 realX, r32 realY, r32 cAlpha)
{
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

void MoveEntity(GameState* gameState, Entity entity, r32 timeToAdvance, V2 playerAcceleration)
{
	World* world = gameState->World;

	r32 playerAccelerationLength = LengthSq(playerAcceleration);

	if (playerAccelerationLength > 1.0f)
	{
		playerAcceleration *= (1.0f / SquareRoot(playerAccelerationLength));
	}

	r32 PlayerSpeed = 50.0f; // m/s^2
	playerAcceleration *= PlayerSpeed;

	// TODO: ODE here!
	playerAcceleration += -8.0f * entity.High->Velocity;

	V2 oldPlayerPosition = entity.High->Position;
	V2 playerDelta = (0.5f * playerAcceleration * Square(timeToAdvance) + entity.High->Velocity * timeToAdvance);

	entity.High->Velocity = playerAcceleration * timeToAdvance + entity.High->Velocity;

	V2 newPlayerPosition = oldPlayerPosition + playerDelta;

	for (u32 iteration = 0; iteration < 4; iteration++)
	{
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
					r32 diameterHeight = testEntity.Low->Height + entity.Low->Height;

					V2 minCorner = -0.5f * V2{ diameterWidth, diameterHeight };
					V2 maxCorner = 0.5f * V2{ diameterWidth, diameterHeight };

					V2 relDifference = entity.High->Position - testEntity.High->Position;

					if (TestWall(minCorner.X, relDifference.X, relDifference.Y, playerDelta.X, playerDelta.Y, &tMin, minCorner.Y, maxCorner.Y))
					{
						wallNormal = V2{ -1, 0 };
						hitHighEntityIndex = testHighEntityIndex;
					}

					if (TestWall(maxCorner.X, relDifference.X, relDifference.Y, playerDelta.X, playerDelta.Y, &tMin, minCorner.Y, maxCorner.Y))
					{
						wallNormal = V2{ 1, 0 };
						hitHighEntityIndex = testHighEntityIndex;
					}

					if (TestWall(minCorner.Y, relDifference.Y, relDifference.X, playerDelta.Y, playerDelta.X, &tMin, minCorner.X, maxCorner.X))
					{
						wallNormal = V2{ 0, -1 };
						hitHighEntityIndex = testHighEntityIndex;
					}

					if (TestWall(maxCorner.Y, relDifference.Y, relDifference.X, playerDelta.Y, playerDelta.X, &tMin, minCorner.X, maxCorner.X))
					{
						wallNormal = V2{ 0, 1 };
						hitHighEntityIndex = testHighEntityIndex;
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

			HighEntity* HitHigh = gameState->HighEntities + hitHighEntityIndex;
			LowEntity* HitLow = gameState->LowEntities + HitHigh->LowEntityIndex;
			// TODO: stairs
			// entity.High->AbsTileZ += HitLow->dAbsTileZ;
		}
		else
		{
			break;
		}
	}

	// TODO: Change to using the acceleration vector
	if ((entity.High->Velocity.X == 0.0f) && (entity.High->Velocity.Y == 0.0f))
	{
		// NOTE: Leave FacingDirection whatever it was
	}
	else if (AbsoluteValue(entity.High->Velocity.X) > AbsoluteValue(entity.High->Velocity.Y))
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
	else
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

	WorldPosition newPosition = MapIntoChunkSpace(world, gameState->CameraPosition, entity.High->Position);
	ChangeEntityLocation(&gameState->WorldMemoryPool, gameState->World, entity.LowEntityIndex, &entity.Low->Position, &newPosition);
	entity.Low->Position = newPosition;
}

void InitializePlayer(GameState* state)
{
	AddLowEntityResult result = AddLowEntity(state, EntityType::Player, &state->CameraPosition);
	state->PlayerEntity = { result.LowEntityIndex, result.LowEntity };

	Entity* entity = &state->PlayerEntity;

	// Order: X Y Z Offset
	entity->Low->MaxHp = 3;
	entity->Low->Hitpoints[2].Current = HitPointMaxAmount;
	entity->Low->Hitpoints[0].Current = entity->Low->Hitpoints[1].Current = entity->Low->Hitpoints[2].Current;
	entity->Low->Collides = true;
	entity->Low->Height = 1.0f;
	entity->Low->Width = 0.75f;
	entity->Low->Speed = 30.0f; // M/S2

	MakeEntityHighFreq(state, result.LowEntityIndex);

	entity->High = state->HighEntities + entity->Low->HighEntityIndex;
}

AddLowEntityResult AddMonster(GameState* state, WorldPosition* position)
{
	AddLowEntityResult result = AddLowEntity(state, EntityType::Monster, position);

	// Order: X Y Z Offset
	result.LowEntity->Collides = true;
	result.LowEntity->Height = 0.5f;
	result.LowEntity->Width = 1.0f;
	result.LowEntity->Speed = 20.0f; // M/S2

	return result;
}
AddLowEntityResult AddFamiliar(GameState* state, WorldPosition* position)
{
	AddLowEntityResult result = AddLowEntity(state, EntityType::Familiar, position);

	// Order: X Y Z Offset
	result.LowEntity->Collides = false;
	result.LowEntity->Height = 0.5f;
	result.LowEntity->Width = 1.0f;
	result.LowEntity->Speed = 10.0f;

	return result;
}

AddLowEntityResult AddLowEntity(GameState* gameState, EntityType type, WorldPosition* position)
{
	u32 entityIndex = 0;
	LowEntity* lowEnttiy = 0;

	if (gameState->LowEntitiesCount < ArrayCount(gameState->LowEntities))
	{
		entityIndex = gameState->LowEntitiesCount++;

		lowEnttiy = gameState->LowEntities + entityIndex;

		*lowEnttiy = {};
		lowEnttiy->Type = type;

		if (position)
		{
			(*lowEnttiy).Position = *position;
			ChangeEntityLocation(&gameState->WorldMemoryPool, gameState->World, entityIndex, 0, position);
		}
	}

	return { entityIndex, lowEnttiy };
}

AddLowEntityResult AddWall(GameState* gameState, i32 x, i32 y, i32 z)
{
	WorldPosition pos = GetChunkPositionFromWorldPosition(gameState->World, x, y, z);

	AddLowEntityResult result = AddLowEntity(gameState, EntityType::Wall, &pos);

	// Order: X Y Z Offset
	result.LowEntity->Collides = true;
	result.LowEntity->Height = gameState->World->TileSideInMeters;
	result.LowEntity->Width = result.LowEntity->Height;

	return result;
}

inline V2 GetCameraSpacePosition(GameState* gameState, LowEntity* lowEntity)
{
	V3 diff = CalculatePositionDifference(gameState->World, &lowEntity->Position, &gameState->CameraPosition);
	V2 resutlt = V2{ diff.X, diff.Y };
	return resutlt;
}

HighEntity* MakeEntityHighFreq(GameState* gameState, LowEntity* lowEntity, u32 index, V2 camerSpaceP)
{
	HighEntity* highEntity = 0;

	Assert(!lowEntity->HighEntityIndex);

	if (gameState->HighEntitiesCount < ArrayCount(gameState->HighEntities))
	{
		u32 highEntityIndex = gameState->HighEntitiesCount++;
		highEntity = gameState->HighEntities + highEntityIndex;

		highEntity->Position = camerSpaceP;
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

	return highEntity;
}

HighEntity* MakeEntityHighFreq(GameState* gameState, u32 lowEntityIndex)
{
	LowEntity* lowEntity = gameState->LowEntities + lowEntityIndex;

	HighEntity* highEntity = 0;

	if (lowEntity->HighEntityIndex)
	{
		highEntity = gameState->HighEntities + lowEntity->HighEntityIndex;
	}
	else
	{
		V2 caperaSpacePosition = GetCameraSpacePosition(gameState, lowEntity);
		highEntity = MakeEntityHighFreq(gameState, lowEntity, lowEntityIndex, caperaSpacePosition);
	}

	return highEntity;
}

void MakeEntityLowFreq(GameState* gameState, u32 index)
{
	LowEntity* lowEntity = &gameState->LowEntities[index];
	u32 highEntityIndex = lowEntity->HighEntityIndex;

	if (highEntityIndex)
	{
		u32 lastHighEntityIndex = gameState->HighEntitiesCount - 1;

		if (highEntityIndex != lastHighEntityIndex)
		{
			HighEntity* lastHighEntity = gameState->HighEntities + lastHighEntityIndex;
			HighEntity* deletedEntity = gameState->HighEntities + highEntityIndex;

			*deletedEntity = *lastHighEntity;
			gameState->LowEntities[lastHighEntity->LowEntityIndex].HighEntityIndex = highEntityIndex;
		}

		--gameState->HighEntitiesCount;
		lowEntity->HighEntityIndex = 0;
	}
}

void SetCamera(GameState* gameState, WorldPosition newPosition)
{
	World* world = gameState->World;

	Assert(ValidateEntityPairs(gameState));

	V3 dCamera = CalculatePositionDifference(world, &newPosition, &gameState->CameraPosition);
	gameState->CameraPosition = newPosition;

	// Its negative so everything is move against the camera, now with the camera.
	V2 entityOffsetForFrame = -V2{ dCamera.X, dCamera.Y };

	// 3 screens in each direction
	r32 tileSpanX = 17.0f * 3.0f;
	r32 tileSpanY = 9.0f * 3.0f;

	R2 cameraBounds = RectCenterDim(V2{ 0, 0 }, world->TileSideInMeters * V2{ tileSpanX, tileSpanY });

	OffsetAndCheckFrequencyByArea(gameState, entityOffsetForFrame, cameraBounds);

	Assert(ValidateEntityPairs(gameState));

	WorldPosition minChunkPos = MapIntoChunkSpace(world, newPosition, GetMinCorner(cameraBounds));
	WorldPosition maxChunkPos = MapIntoChunkSpace(world, newPosition, GetMaxCorner(cameraBounds));

	for (i32 chunkY = minChunkPos.Y; chunkY <= maxChunkPos.Y; chunkY++)
	{
		for (i32 chunkX = minChunkPos.X; chunkX <= maxChunkPos.X; chunkX++)
		{
			WorldChunk* chunk = GetWorldChunk(world, chunkX, chunkY, newPosition.Z);

			if (chunk)
			{
				for (EntityBlock* block = &chunk->FirstBlock; block; block = block->Next)
				{
					for (u32 entityIndex = 0; entityIndex < block->EntitiesCount; entityIndex++)
					{
						u32 lowEntityIndex = block->LowEntitiyIndex[entityIndex];

						LowEntity* lowEntity = gameState->LowEntities + lowEntityIndex;

						if (!lowEntity->HighEntityIndex)
						{
							V2 cameraSpaceP = GetCameraSpacePosition(gameState, lowEntity);

							if (IsInRectangle(cameraBounds, cameraSpaceP))
							{
								MakeEntityHighFreq(gameState, lowEntity, lowEntityIndex, cameraSpaceP);
							}
						}
					}
				}
			}
		}
	}

	Assert(ValidateEntityPairs(gameState));
}

b32 TestWall(r32 wall, r32 relX, r32 relY, r32 playerDeltaX, r32 playerDeltaY, r32* tMin, r32 minY, r32 maxY)
{
	b32 hitWall = false;
	r32 tEpslion = 0.0001f;

	if (playerDeltaX != 0)
	{
		r32 tResult = (wall - relX) / playerDeltaX;

		r32 y = relY + tResult * playerDeltaY;

		if ((tResult >= 0.0f) && (*tMin > tResult))
		{
			if ((y >= minY) && (y <= maxY))
			{
				*tMin = Maximum(0.0f, tResult - tEpslion);
				hitWall = true;
			}
		}
	}

	return hitWall;
}

inline void PushPiece(EntityVisiblePieceGroup* group, LoadedBitmap* bitmap, V2 offset, r32 offsetZ, V2 dim, V4 colour, V2 align, r32 zCoefficient)
{
	Assert(group->PieceCount < ArrayCount(group->Pieces));

	EntityVisiblePiece* piece = group->Pieces + group->PieceCount++;

	piece->Bitmap = bitmap;
	piece->Offset = group->GameState->MetersToPixels * V2{ offset.X, -offset.Y } - align;
	piece->Z = group->GameState->MetersToPixels * offsetZ;
	piece->ZCoefficient = zCoefficient;
	piece->Dimensions = dim;
	piece->Colour = colour;
}

inline void PushBitmap(EntityVisiblePieceGroup* group, LoadedBitmap* bitmap, V2 offset, r32 offsetZ, V2 align, r32 alpha, r32 zCoefficient)
{
	PushPiece(group, bitmap, offset, offsetZ, {}, { 0.0f, 0.0f, 0.0f, alpha }, align, zCoefficient);
}

inline void PushRect(EntityVisiblePieceGroup* group, V2 offset, r32 offsetZ, V2 dim, V4 colour, r32 zCoefficient)
{
	PushPiece(group, 0, offset, offsetZ, dim, colour, {}, zCoefficient);
}

void UpdateFamiliar(GameState* state, Entity entity, r32 timeDelta)
{
	r32 maximumSearchRadius = Square(5.0f);

	Entity player = state->PlayerEntity;

	r32 testSq = LengthSq(player.High->Position - entity.High->Position);

	V2 ddP = {};

	if (testSq < maximumSearchRadius && testSq > 0.01f)
	{
		// TODO: PULL SPEED OUT OF MOVE ENTITY
		r32 acceleration = 0.5f;

		r32 oneOverLength = acceleration / SquareRoot(testSq);

		ddP = oneOverLength * (player.High->Position - entity.High->Position);
	}

	MoveEntity(state, entity, timeDelta, ddP);
}

void UpdateMonster(GameState* state, Entity entity, r32 dt)
{

}