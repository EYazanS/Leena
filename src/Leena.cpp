#include "Leena.h"
#include "World/World.cpp"
#include "RandomNumbers.h"
#include "SimRegion/SimRegion.cpp"
#include "Entities/Enitites.cpp"

AudioBuffer *ReadAudioBufferData(void *memory)
{
	u8 *byte = (u8 *)memory; // Get the firs 4 bytes of the memory

	// Move to position 21
	byte += 20;

	AudioBuffer *result = (AudioBuffer *)byte;

	// Move to Data chunk
	char *characterToRead = ((char *)byte);

	characterToRead += sizeof(AudioBuffer);

	while (*characterToRead != 'd' || *(characterToRead + 1) != 'a' || *(characterToRead + 2) != 't' || *(characterToRead + 3) != 'a')
	{
		characterToRead++;
	}

	characterToRead += 4;

	byte = (u8 *)(characterToRead);

	u32 bufferSize = *((u32 *)byte);

	byte += 4;

	// Get one third of a sec
	u32 totalBytesNeeded = (u32)(result->SamplesPerSec * 0.5f);

	result->BufferSize = totalBytesNeeded;
	result->BufferData = byte;

	return result;
}

void FillAudioBuffer(ThreadContext *thread, GameMemory *gameMemory, AudioBuffer *soundBuffer)
{
	DebugFileResult file = gameMemory->ReadFile(thread, "resources/Water_Splash_SeaLion_Fienup_001.wav");

	if (file.Memory)
	{
		AudioBuffer *result = ReadAudioBufferData(file.Memory);

		*soundBuffer = *result;
	}
}

void DrawRectangle(
	ScreenBuffer *gameScreenBuffer,
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

	u8 *endOfBuffer = ((u8 *)gameScreenBuffer->Memory) + ((u64)gameScreenBuffer->Pitch * (u64)gameScreenBuffer->Height);

	u8 *row = (((u8 *)gameScreenBuffer->Memory) + (i64)minX * gameScreenBuffer->BytesPerPixel + (i64)minY * gameScreenBuffer->Pitch);

	for (i32 y = minY; y < maxY; ++y)
	{
		u32 *pixel = (u32 *)row;

		for (i32 x = minX; x < maxX; ++x)
		{
			*pixel++ = finalColour;
		}

		row += gameScreenBuffer->Pitch;
	}
}

void DrawBitmap(ScreenBuffer *screenBuffer, LoadedBitmap *bitmap, r32 realX, r32 realY, r32 cAlpha)
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

	u32 *sourceRow = bitmap->Data + bitmap->Width * (bitmap->Height - 1);
	sourceRow += (-sourceOffsetY * bitmap->Width) + sourceOffsetX;

	u8 *destRow = (((u8 *)screenBuffer->Memory) + minX * screenBuffer->BytesPerPixel + minY * screenBuffer->Pitch);

	for (i64 y = minY; y < maxY; y++)
	{
		u32 *dest = (u32 *)destRow;
		u32 *source = sourceRow;

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

LoadedBitmap DebugLoadBmp(ThreadContext *thread, PlatformReadEntireFile *readFile, const char *fileName)
{
	LoadedBitmap result = {};

	DebugFileResult bmp = readFile(thread, fileName);

	if (bmp.FileSize != 0)
	{
		BitmapHeader *header = (BitmapHeader *)bmp.Memory;

		u32 *pixels = (u32 *)((u8 *)bmp.Memory + header->BitmapOffset);

		u32 *source = pixels;

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

inline V2 GetCameraSpacePosition(GameState *gameState, LowEntity *lowEntity)
{
	V3 diff = CalculatePositionDifference(gameState->World, &lowEntity->Position, &gameState->CameraPosition);
	V2 resutlt = V2{diff.X, diff.Y};
	return resutlt;
}

struct AddLowEntityResult
{
	u32 LowEntityIndex;
	LowEntity *LowEntity;
};

AddLowEntityResult AddLowEntity(GameState *GameState, EntityType Type, WorldPosition P)
{
	Assert(GameState->LowEntitiesCount < ArrayCount(GameState->LowEntities));
	u32 EntityIndex = GameState->LowEntitiesCount++;

	LowEntity *EntityLow = GameState->LowEntities + EntityIndex;
	*EntityLow = {};
	EntityLow->Entity.Type = Type;
	EntityLow->Position = NullPosition();

	ChangeEntityLocation(&GameState->WorldMemoryPool, GameState->World, EntityIndex, EntityLow, P);

	AddLowEntityResult Result;
	Result.LowEntity = EntityLow;
	Result.LowEntityIndex = EntityIndex;

	// TODO(casey): Do we need to have a begin/end paradigm for adding
	// entities so that they can be brought into the high set when they
	// are added and are in the camera region?

	return (Result);
}

AddLowEntityResult AddWall(GameState *gameState, i32 x, i32 y, i32 z)
{
	WorldPosition pos = GetChunkPositionFromWorldPosition(gameState->World, x, y, z);

	AddLowEntityResult result = AddLowEntity(gameState, EntityType::Wall, pos);

	// Order: X Y Z Offset
	AddFlag(&result.LowEntity->Entity, EntityFlag::Collides);

	result.LowEntity->Entity.Height = gameState->World->TileSideInMeters;
	result.LowEntity->Entity.Width = result.LowEntity->Entity.Height;

	return result;
}

void InitializeHitpoints(SimEntity *entity, u32 hitpointsCount)
{
	Assert(hitpointsCount <= ArrayCount(entity->Hitpoints));
	entity->MaxHp = hitpointsCount;

	for (u32 hitpointIndex = 0; hitpointIndex < entity->MaxHp; hitpointIndex++)
	{
		entity->Hitpoints[hitpointIndex].Current = HitPointMaxAmount;
	}
}

AddLowEntityResult AddSword(GameState *state)
{
	AddLowEntityResult result = AddLowEntity(state, EntityType::Sword, NullPosition());

	// Order: X Y Z Offset
	result.LowEntity->Entity.Height = 0.5f;
	result.LowEntity->Entity.Width = 1.0f;

	return result;
}

void InitializePlayer(GameState *state)
{
	AddLowEntityResult result = AddLowEntity(state, EntityType::Player, state->CameraPosition);

	SimEntity *entity = &result.LowEntity->Entity;

	// Order: X Y Z Offset
	InitializeHitpoints(entity, 5);
	AddFlag(entity, EntityFlag::Collides);

	entity->Height = 1.0f;
	entity->Width = 0.75f;
	entity->Speed = 20.0f; // M/S2

	AddLowEntityResult sword = AddSword(state);

	entity->SwordLowIndex.Index = sword.LowEntityIndex;
	state->PlayerEntity = entity;
	state->PlayerEntity->StorageIndex = result.LowEntityIndex;
}

AddLowEntityResult AddMonster(GameState *state, WorldPosition position)
{
	AddLowEntityResult result = AddLowEntity(state, EntityType::Monster, position);

	// Order: X Y Z Offset
	InitializeHitpoints(&result.LowEntity->Entity, 3);

	AddFlag(&result.LowEntity->Entity, EntityFlag::Collides);
	result.LowEntity->Entity.Height = 0.5f;
	result.LowEntity->Entity.Width = 1.0f;
	result.LowEntity->Entity.Speed = 5.0f; // M/S2

	return result;
}

AddLowEntityResult AddFamiliar(GameState *state, WorldPosition position)
{
	AddLowEntityResult result = AddLowEntity(state, EntityType::Familiar, position);

	// Order: X Y Z Offset
	result.LowEntity->Entity.Height = 0.5f;
	result.LowEntity->Entity.Width = 1.0f;
	result.LowEntity->Entity.Speed = 5.0f;

	return result;
}

void SimCameraRegion(GameState *gameState)
{
}

inline void PushPiece(EntityVisiblePieceGroup *group, LoadedBitmap *bitmap, V2 offset, r32 offsetZ, V2 dim, V4 colour, V2 align, r32 zCoefficient)
{
	Assert(group->PieceCount < ArrayCount(group->Pieces));

	EntityVisiblePiece *piece = group->Pieces + group->PieceCount++;

	piece->Bitmap = bitmap;
	piece->Offset = group->GameState->MetersToPixels * V2{offset.X, -offset.Y} - align;
	piece->Z = group->GameState->MetersToPixels * offsetZ;
	piece->ZCoefficient = zCoefficient;
	piece->Dimensions = dim;
	piece->Colour = colour;
}

inline void PushBitmap(EntityVisiblePieceGroup *group, LoadedBitmap *bitmap, V2 offset, r32 offsetZ, V2 align, r32 alpha = 1.0f, r32 zCoefficient = 1.0f)
{
	PushPiece(group, bitmap, offset, offsetZ, {}, {0.0f, 0.0f, 0.0f, alpha}, align, zCoefficient);
}

inline void PushRect(EntityVisiblePieceGroup *group, V2 offset, r32 offsetZ, V2 dim, V4 colour, r32 zCoefficient)
{
	PushPiece(group, 0, offset, offsetZ, dim, colour, {}, zCoefficient);
}

void DrawHitPoints(SimEntity *entity, EntityVisiblePieceGroup *pieceGroup)
{
	if (entity->MaxHp > 0)
	{
		V2 healthDim = {0.2f, 0.2f};
		r32 spacing = healthDim.X * 1.5f;

		V2 hpPosition = {-0.5f * (entity->MaxHp - 1) * spacing, -0.40f};
		V2 dHitP = {spacing, 0.0f};

		for (u8 healthIndex = 0; healthIndex < entity->MaxHp; healthIndex++)
		{
			Hitpoint *hitPoint = entity->Hitpoints + healthIndex;

			V4 colour = {1.0f, 0.0f, 0.0f, 1.0f};

			if (hitPoint->Current == 0)
			{
				colour.R = 0.2f;
				colour.G = 0.2f;
				colour.B = 0.2f;
			}

			PushRect(pieceGroup, hpPosition, 0, healthDim, colour, 0.0f);

			hpPosition += dHitP;
		}
	}
}

DllExport void GameUpdateAndRender(ThreadContext *thread, GameMemory *gameMemory, ScreenBuffer *screenBuffer, GameInput *input)
{
	GameState *gameState = (GameState *)gameMemory->PermanentStorage;

	if (!gameMemory->IsInitialized)
	{
		initializePool(&gameState->WorldMemoryPool, gameMemory->PermanentStorageSize - sizeof(GameState), (u8 *)gameMemory->PermanentStorage + sizeof(GameState));

		gameState->World = PushStruct(&gameState->WorldMemoryPool, World);

		World *world = gameState->World;

		initializeWorld(world);

		gameState->Background = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_background.bmp");
		gameState->Tree = DebugLoadBmp(thread, gameMemory->ReadFile, "test2/tree00.bmp");
		gameState->Rock = DebugLoadBmp(thread, gameMemory->ReadFile, "test2/rock00.bmp");
		gameState->Sword = DebugLoadBmp(thread, gameMemory->ReadFile, "test2/rock03.bmp");

		PlayerBitMap *playerBitMap;
		playerBitMap = gameState->BitMaps;

		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_right_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->Align = {72, 182};

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_back_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->Align = {72, 182};

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_left_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->Align = {72, 182};

		playerBitMap++;
		playerBitMap->Head = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_head.bmp");
		playerBitMap->Cape = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_cape.bmp");
		playerBitMap->Torso = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_front_torso.bmp");
		playerBitMap->Shadow = DebugLoadBmp(thread, gameMemory->ReadFile, "test/test_hero_shadow.bmp");
		playerBitMap->Align = {72, 182};

		AddLowEntity(gameState, EntityType::Null, NullPosition());

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

		gameState->CameraPosition = cameraPosition;

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
		// b32 doorUp = false;
		// b32 doorDown = false;

		for (u32 screenIndex = 0; screenIndex < 50; ++screenIndex)
		{
			Assert(randomNumberIndex < ArrayCount(randomNumberTable));

			u32 randomChoice;

			randomChoice = randomNumberTable[randomNumberIndex++] % 2;
			// if (doorUp || doorDown)
			//{
			// }
			// else
			//{
			//	randomChoice = randomNumberTable[sandomNumberIndex++] % 3;
			// }

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
			else*/
			if (randomChoice == 1)
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

					// if (tileX == 10 && tileY == 6)
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
			else */
			if (randomChoice == 1)
			{
				screenX++;
			}
			else
			{
				screenY++;
			}
		}

		WorldPosition monsterPost = GetChunkPositionFromWorldPosition(world, CameraTileX + 2, CameraTileY + 2, CameraTileZ);

		AddMonster(gameState, monsterPost);

		for (int FamiliarIndex = 0; FamiliarIndex < 5; ++FamiliarIndex)
		{
			i32 FamiliarOffsetX = (randomNumberTable[randomNumberIndex++] % 10) - 7;
			i32 FamiliarOffsetY = (randomNumberTable[randomNumberIndex++] % 10) - 3;

			if ((FamiliarOffsetX != 0) || (FamiliarOffsetY != 0))
			{
				WorldPosition familiarPosition = GetChunkPositionFromWorldPosition(world, CameraTileX + FamiliarOffsetX, CameraTileY + FamiliarOffsetY, CameraTileZ);

				AddFamiliar(gameState, familiarPosition);
			}
		}

		i32 tileSideInPixels = 60;

		gameState->MetersToPixels = (r32)tileSideInPixels / (r32)world->TileSideInMeters;

		gameMemory->IsInitialized = true;
	}

	World *world = gameState->World;
	SimEntity *player = gameState->PlayerEntity;

	ControlRequest controlRequest = {};

	//
	// Handle input
	//
	KeyboardInput *keyboard = &input->Keyboard;

	ControllerInput *controller = &input->Controller;

	if (input->States[(int)KeyAction::MoveRight].EndedDown)
	{
		controlRequest.Acceleration.X = 1.0f;
	}
	if (input->States[(int)KeyAction::MoveUp].EndedDown)
	{
		controlRequest.Acceleration.Y = 1.0f;
	}
	if (input->States[(int)KeyAction::MoveDown].EndedDown)
	{
		controlRequest.Acceleration.Y = -1.0f;
	}
	if (input->States[(int)KeyAction::MoveLeft].EndedDown)
	{
		controlRequest.Acceleration.X = -1.0f;
	}

	if (controller->IsConnected)
	{
		if (controller->IsAnalog)
		{
			if (controller->LeftStickAverageX || controller->LeftStickAverageY)
			{
				// NOTE: Use analog movement tuning
				controlRequest.Acceleration = {controller->LeftStickAverageX, controller->LeftStickAverageY};
			}
		}
	}

	if ((input->States[(int)KeyAction::Jump].EndedDown) && player->Z == 0.0f)
	{
		controlRequest.Dz = 3.0f;
	}

	if (input->States[(int)KeyAction::X].EndedDown)
	{
		if (input->States[(int)KeyAction::MoveUp].EndedDown)
		{
			controlRequest.SwordAcceleration = {0.0f, 1.0f};
		}
		if (input->States[(int)KeyAction::MoveDown].EndedDown)
		{
			controlRequest.SwordAcceleration = {0.0f, -1.0f};
		}

		if (input->States[(int)KeyAction::MoveLeft].EndedDown)
		{
			controlRequest.SwordAcceleration = {-1.0f, 0.0f};
		}

		if (input->States[(int)KeyAction::MoveRight].EndedDown)
		{
			controlRequest.SwordAcceleration = {1.0f, 0.0f};
		}
	}

	// if (input->States[(int)KeyAction::Start].EndedDown)
	// {
	// 	gameMemory->IsInitialized = false;
	// }

	if (input->States[(int)KeyAction::Run].EndedDown)
	{
		player->Speed = 60.0f;
	}
	else
	{
		player->Speed = 30.0f;
	}

	r32 tileSpanX = 17.0f * 3.0f;
	r32 tileSpanY = 9.0f * 3.0f;

	R2 cameraBounds = RectCenterDim(V2{0, 0}, world->TileSideInMeters * V2{tileSpanX, tileSpanY});

	MemoryPool simArena;

	initializePool(&simArena, gameMemory->TransiateStorageSize, gameMemory->TransiateStorage);

	SimRegion *simRegion = BeginSim(
		&simArena,
		gameState,
		gameState->World,
		cameraBounds,
		gameState->CameraPosition);

	//
	// Draw New game status
	//

	r32 metersToPixels = gameState->MetersToPixels;

	V2 screenCenter = 0.5f * V2{(r32)screenBuffer->Width, (r32)screenBuffer->Height};

#if 1
	DrawRectangle(screenBuffer, {}, {(r32)screenBuffer->Width, (r32)screenBuffer->Height}, {0.5f, 0.5f, 0.5f});
#else
	DrawBitmap(&gameState->Background, screenBuffer, 0, 0);
#endif // 0

	EntityVisiblePieceGroup pieceGroup = {};

	pieceGroup.GameState = gameState;

	SimEntity *entity = simRegion->Entities;

	// Render Entities
	for (u32 entityIndex = 0; entityIndex < simRegion->EntitiesCount; entityIndex++, ++entity)
	{
		if (!entity->Updatable)
		{
			continue;
		}

		pieceGroup.PieceCount = 0;

		r32 dt = (r32)input->TimeToAdvance;

		// TODO: fix
		r32 shadowAlpha = 1.0f - 0.5f * entity->Z;

		if (shadowAlpha < 0)
		{
			shadowAlpha = 0.0f;
		}

		PlayerBitMap *playerFacingDirectionMap = &gameState->BitMaps[entity->FacingDirection];

		switch (entity->Type)
		{
		case EntityType::Player:
		{
			if (controlRequest.Dz)
			{
				entity->dZ = controlRequest.Dz;
			}

			MoveSpec moveSpec = GetDefaultMoveSpec();
			moveSpec.UnitMaxAccVector = true;
			moveSpec.Speed = entity->Speed;
			moveSpec.Drag = 8.0f;

			MoveEntity(simRegion, entity, (r32)input->TimeToAdvance, controlRequest.Acceleration, &moveSpec);

			if (controlRequest.SwordAcceleration.X != 0.0f || controlRequest.SwordAcceleration.Y != 0.0f)
			{
				SimEntity *sword = entity->SwordLowIndex.Ptr;

				if (sword && HasFlag(sword, EntityFlag::Nonspatial))
				{
					sword->DistanceRemaining = 5.0f;
					MakeEntitySpatial(sword, entity->Position, 5.0f * controlRequest.SwordAcceleration);
				}
			}

			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Head, V2{0, 0}, 0, playerFacingDirectionMap->Align);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Cape, V2{0, 0}, 0, playerFacingDirectionMap->Align);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Torso, V2{0, 0}, 0, playerFacingDirectionMap->Align);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Shadow, V2{0, 0}, 0, playerFacingDirectionMap->Align, shadowAlpha, 0);

			DrawHitPoints(entity, &pieceGroup);
		}
		break;

		case EntityType::Wall:
		{
			PushBitmap(&pieceGroup, &gameState->Tree, V2{0, 0}, 0, V2{40, 80});
		}
		break;

		case EntityType::Sword:
		{
			UpdateSword(simRegion, entity, dt);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Shadow, V2{0, 0}, 0, playerFacingDirectionMap->Align, shadowAlpha, 0);
			PushBitmap(&pieceGroup, &gameState->Sword, V2{0, 0}, 0, V2{29, 10});
		}
		break;

		case EntityType::Familiar:
		{
			UpdateFamiliar(simRegion, entity, dt);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Head, V2{0, 0}, 0, playerFacingDirectionMap->Align);
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Shadow, V2{0, 0}, 0, playerFacingDirectionMap->Align, shadowAlpha, 0);
		}
		break;

		case EntityType::Monster:
		{
			UpdateMonster(simRegion, entity, dt);
			PushBitmap(&pieceGroup, &gameState->Rock, V2{0, 0}, 0, V2{40, 80});
			PushBitmap(&pieceGroup, &playerFacingDirectionMap->Shadow, V2{0, 0}, 0, playerFacingDirectionMap->Align, shadowAlpha);

			DrawHitPoints(entity, &pieceGroup);
		}
		break;

		default:
		{
			InvalidCodePath;
		}
		break;
		}

		// Gravity
		r32 ddZ = -9.8f;

		entity->Z = 0.5f * ddZ * Square(dt) + entity->dZ * dt + entity->Z;

		entity->dZ = ddZ * dt + entity->dZ;

		if (entity->Z < 0)
		{
			entity->Z = 0;
		}

		r32 groundPointX = screenCenter.X + metersToPixels * entity->Position.X;
		r32 groundPointY = screenCenter.Y - metersToPixels * entity->Position.Y;
		r32 entityZ = -metersToPixels * entity->Z;

#if 0
		V2 playerLeftTop = { playerGroundPointX - 0.5f * metersToPixels * LowEntity->Entity.Width, playerGroundPointY - 0.5f * metersToPixels * LowEntity->Entity.Height };
		V2 entityWidthHeight = { LowEntity->Entity.Width, LowEntity->Entity.Height };
		DrawRectangle(screenBuffer, playerLeftTop, playerLeftTop + metersToPixels * entityWidthHeight, { 1.0f, 1.0f, 0.0f });
#endif

		for (u32 pieceIndex = 0; pieceIndex < pieceGroup.PieceCount; pieceIndex++)
		{
			EntityVisiblePiece *piece = pieceGroup.Pieces + pieceIndex;

			V2 center = {groundPointX + piece->Offset.X, groundPointY + piece->Offset.Y + piece->Z + piece->ZCoefficient * entityZ};

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

	EndSim(simRegion, gameState);
}

DllExport void GameUpdateAudio(ThreadContext *thread, GameMemory *gameMemory, AudioBuffer *audioBuffer)
{
#if 0
	FillAudioBuffer(thread, gameMemory, audioBuffer);
#endif // 0
}