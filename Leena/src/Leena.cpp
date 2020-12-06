#include "Leena.h"

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int PlayerX, int PlayerY);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, uint64 playerX, uint64 playerY, real32 playerWidth, real32 playerHeight);
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);

void DrawRectangle(
	GameScreenBuffer* gameScreenBuffer,
	real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY,
	Colour colour);
void DrawTileMap(World* world, GameScreenBuffer* screenBuffer, TileMap* tileMap);
int32 IsTileMapPointEmpty(World* world, TileMap* tileMap, int32 tileTestX, int32 tileTestY);
bool32 IsWorldPointEmpty(World* world, RawLocation location);
inline int32 GetTileValueUnchecked(TileMap* tileMap, size_t tileCountX, size_t tileX, size_t tileY);
inline TileMap* GetTileMap(World* world, size_t tileX, size_t tileY);
inline CononicalLocation Canoniocalize(World* world, RawLocation location);

void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	if (!gameMemory->IsInitialized)
	{
		gameState->PlayerX = 140;
		gameState->PlayerY = 70;
		gameState->PlayerTileMapX = 0;
		gameState->PlayerTileMapY = 0;
		gameMemory->IsInitialized = true;
	}

	TileMap tileMaps[2][2] = {};

	World world = {};

	world.TileMapCountX = 2;
	world.TileMapCountY = 2;

	world.TileMapSideInMeter = 1.4f;

	world.TileMaps = (TileMap*)&tileMaps;
	world.CountX = 16;
	world.CountY = 9;

	world.TileSideInPixels = 60;

	world.UpperLeftX = -world.TileSideInPixels / 2;
	world.UpperLeftY = 0;

	uint32 tiles00[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles01[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles10[16][9] =
	{
		1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles11[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	tileMaps[0][0].Tiles = (uint32*)tiles00;
	tileMaps[0][1].Tiles = (uint32*)tiles01;
	tileMaps[1][0].Tiles = (uint32*)tiles10;
	tileMaps[1][1].Tiles = (uint32*)tiles11;

	TileMap* currentTileMap = GetTileMap(&world, gameState->PlayerTileMapX, gameState->PlayerTileMapY);
	Assert(currentTileMap);
	real32 playerWidth = 0.5f * (real32)world.TileSideInPixels;
	real32 playerHeight = 1.25f * (real32)world.TileSideInPixels;

	real32 pixelsToMovePerSec = 250.f;

	real32 playerMovementX = 0.f; // pixels/second
	real32 playerMovementY = 0.f; // pixels/second

	if (input->Keyboard.A.EndedDown)
		playerMovementX += -1.f;

	if (input->Keyboard.D.EndedDown)
		playerMovementX += 1.f;

	if (input->Keyboard.W.EndedDown)
		playerMovementY += -1.f;

	if (input->Keyboard.S.EndedDown)
		playerMovementY += 1.f;

	playerMovementY *= pixelsToMovePerSec;
	playerMovementX *= pixelsToMovePerSec;

	// TODO: Deal with controller movement
	for (const GameControllerInput& controller : input->Controllers)
	{
		if (controller.IsConnected && controller.IsAnalog)
		{
			gameState->PlayerX += static_cast<int>(pixelsToMovePerSec * input->TimeToAdvance * controller.LeftStickAverageX);
			gameState->PlayerY += static_cast<int>(pixelsToMovePerSec * input->TimeToAdvance * controller.LeftStickAverageY);
		}
	}

	real32 newPlayerX = gameState->PlayerX + (playerMovementX * (real32)input->TimeToAdvance);
	real32 newPlayerY = gameState->PlayerY + (playerMovementY * (real32)input->TimeToAdvance);

	RawLocation playerLocation = { gameState->PlayerTileMapX, gameState->PlayerTileMapY, newPlayerX, newPlayerY };

	RawLocation playerLeftLocation = playerLocation;
	playerLeftLocation.PlayerX -= 0.5f * playerWidth;

	RawLocation playerRightLocation = playerLocation;
	playerRightLocation.PlayerX += 0.5f * playerWidth;

	if (IsWorldPointEmpty(&world, playerLocation) && IsWorldPointEmpty(&world, playerLeftLocation) && IsWorldPointEmpty(&world, playerRightLocation))
	{
		CononicalLocation canLocation = Canoniocalize(&world, playerLocation);

		gameState->PlayerTileMapX = canLocation.TileMapX;
		gameState->PlayerTileMapY = canLocation.TileMapY;
		gameState->PlayerX = world.UpperLeftX + world.TileSideInPixels * canLocation.TileX + TruncateReal32ToInt32(canLocation.PlayerX);
		gameState->PlayerY = world.UpperLeftY + world.TileSideInPixels * canLocation.TileY + TruncateReal32ToInt32(canLocation.PlayerY);
	}

	DrawTileMap(&world, screenBuffer, currentTileMap);
	RenderPlayer(screenBuffer, gameState->PlayerX, gameState->PlayerY, playerWidth, playerHeight);
	FillAudioBuffer(thread, gameMemory, soundBuffer);
}

inline CononicalLocation Canoniocalize(World* world, RawLocation location)
{
	CononicalLocation result = {};

	result.TileMapX = location.TileMapX;
	result.TileMapY = location.TileMapY;

	real32 playerX = location.PlayerX - world->UpperLeftX;
	real32 playerY = location.PlayerY - world->UpperLeftY;

	result.TileX = FloorReal32ToInt32(playerX / world->TileSideInPixels);
	result.TileY = FloorReal32ToInt32(playerY / world->TileSideInPixels);

	result.PlayerX = playerX - result.TileX * world->TileSideInPixels;
	result.PlayerY = playerY - result.TileY * world->TileSideInPixels;

	Assert(result.PlayerX >= 0);
	Assert(result.PlayerX < world->TileSideInPixels);
	Assert(result.PlayerY >= 0);
	Assert(result.PlayerY < world->TileSideInPixels);

	if (result.TileX < 0)
	{
		result.TileX = world->CountX + result.TileX;
		--result.TileMapX;
	}

	if (result.TileY < 0)
	{
		result.TileY = world->CountY + result.TileY;
		--result.TileMapY;
	}

	if (result.TileX >= world->CountX)
	{
		result.TileX = world->CountX - result.TileX;
		++result.TileMapX;
	}

	if (result.TileY >= world->CountY)
	{
		result.TileY = world->CountY - result.TileY;
		++result.TileMapY;
	}

	return result;
}

bool32 IsWorldPointEmpty(World* world, RawLocation location)
{
	int32 isEmpty = 0;

	CononicalLocation canonicalLocation = Canoniocalize(world, location);

	TileMap* tileMap = GetTileMap(world, canonicalLocation.TileMapX, canonicalLocation.TileMapY);

	isEmpty = IsTileMapPointEmpty(world, tileMap, canonicalLocation.TileX, canonicalLocation.TileY);

	return isEmpty;
}

int32 IsTileMapPointEmpty(World* world, TileMap* tileMap, int32 testTileX, int32 testTileY)
{
	int32 isEmpty = 0;

	if (tileMap)
	{
		if ((testTileX >= 0 && testTileX < world->CountX) && (testTileY >= 0 && testTileY < world->CountY))
		{
			uint32 tileMapValue = GetTileValueUnchecked(tileMap, world->CountX, testTileX, testTileY);
			isEmpty = tileMapValue == 0;
		}
	}

	return isEmpty;
}

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int xOffset, int yOffset)
{
	uint8* Row = (uint8*)gameScreenBuffer->Memory;

	for (int Y = 0; Y < gameScreenBuffer->Height; ++Y)
	{
		uint32* Pixel = (uint32*)Row;

		for (int X = 0; X < gameScreenBuffer->Width; ++X)
		{
			uint8 g = static_cast<uint8>(Y + yOffset);
			uint8 b = static_cast<uint8>(X + xOffset);

			/*
			Memory:   BB GG RR xx
			Register: xx RR GG BB
			Pixel (32-bits)
			*/

			*Pixel++ = (g << 16) | b;
		}

		Row += gameScreenBuffer->Pitch;
	}
}

void DrawTileMap(World* world, GameScreenBuffer* screenBuffer, TileMap* tileMap)
{
	for (size_t row = 0; row < world->CountY; row++)
	{
		for (size_t column = 0; column < world->CountX; column++)
		{
			Colour colour = {};

			(GetTileValueUnchecked(tileMap, world->CountX, column, row) == 1) ? colour = { 1.f, 1.f, 1.f } : colour = { 0.7f, 0.7f, 0.7f };

			real32 minX = world->UpperLeftX + ((real32)column * world->TileSideInPixels);
			real32 maxX = minX + world->TileSideInPixels;

			real32 minY = world->UpperLeftY + ((real32)row * world->TileSideInPixels);
			real32 maxY = minY + world->TileSideInPixels;

			DrawRectangle(screenBuffer, minX, minY, maxX, maxY, colour);
		}
	}
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

void RenderPlayer(GameScreenBuffer* gameScreenBuffer, uint64 playerX, uint64 playerY, real32 playerWidth, real32 playerHeight)
{
	Colour colour = { 1.f, 0.f, 1.f };

	real32 playerTop = playerY - playerHeight;
	real32 playerLeft = playerX - (0.5f * playerWidth);

	DrawRectangle(gameScreenBuffer, playerLeft, playerTop, playerLeft + playerWidth, playerTop + playerHeight, colour);
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

inline int32 GetTileValueUnchecked(TileMap* tileMap, size_t tileCountX, size_t tileX, size_t tileY)
{
	return tileMap->Tiles[tileY * tileCountX + tileX];
}

inline TileMap* GetTileMap(World* world, size_t tileX, size_t tileY)
{
	TileMap* tileMap = 0;

	if ((tileX >= 0 && tileX <= world->TileMapCountX) && (tileY >= 0 && tileY <= world->TileMapCountY))
		tileMap = &world->TileMaps[tileY * world->TileMapCountX + tileX];

	return tileMap;
}