#include "Leena.h"

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int PlayerX, int PlayerY);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, uint64 playerX, uint64 playerY, real32 playerWidth, real32 playerHeight);
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);
inline int32 RoundReal32ToInt32(real32 value);
inline int32 TruncateReal32ToInt32(real32 value);
void DrawRectangle(
	GameScreenBuffer* gameScreenBuffer,
	real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY,
	Colour colour);
void DrawTimeMap(GameScreenBuffer* screenBuffer, TileMap* tileMap);
int32 IsPointEmpty(const real32& testX, const real32& testY, TileMap* tileMap);
int32 IsWorldPointEmpty(World* world, int32 tileMapX, int32 tileMapY, const real32& testX, const real32& testY);
inline int32 GetTileVAlueUnchecked(TileMap* tileMap, size_t tileX, size_t tileY);
inline TileMap* GetTileMap(World* world, size_t tileX, size_t tileY);

void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	World world = {};

	TileMap tileMaps[2][2] = {};

	world.CountX = 2;
	world.CountY = 2;
	world.TileMaps = (TileMap*)&tileMaps;

	uint32 tiles0[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles1[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles2[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles3[16][9] =
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

	tileMaps[0][0].CountX = 16;
	tileMaps[0][0].CountY = 9;
	tileMaps[0][0].TileHeight = 60;
	tileMaps[0][0].TileWidth = 60;
	tileMaps[0][0].UpperLeftX = -30;
	tileMaps[0][0].UpperLeftY = -30;
	tileMaps[0][0].Tiles = (uint32*)tiles0;

	tileMaps[0][1] = tileMaps[0][0];
	tileMaps[0][1].Tiles = (uint32*)tiles1;

	tileMaps[1][0] = tileMaps[0][0];
	tileMaps[1][0].Tiles = (uint32*)tiles2;

	tileMaps[1][1] = tileMaps[0][0];
	tileMaps[1][1].Tiles = (uint32*)tiles3;

	TileMap* currentTileMap = GetTileMap(&world, gameState->PlayerTileMapX, gameState->PlayerTileMapY);
	Assert(currentTileMap);
	real32 playerWidth = 0.5f * (real32)currentTileMap->TileWidth;
	real32 playerHeight = 0.75f * (real32)currentTileMap->TileHeight;

	if (!gameMemory->IsInitialized)
	{
		gameState->PlayerX = 140;
		gameState->PlayerY = 70;
		gameState->PlayerTileMapX = 0;
		gameState->PlayerTileMapY = 0;
		gameMemory->IsInitialized = true;
	}

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

	if (IsPointEmpty(newPlayerX, newPlayerY, currentTileMap) &&
		IsPointEmpty(newPlayerX - (0.5f * playerWidth), newPlayerY, currentTileMap) &&
		IsPointEmpty(newPlayerX + (0.5f * playerWidth), newPlayerY, currentTileMap)
		)
	{
		gameState->PlayerX = TruncateReal32ToInt32(newPlayerX);
		gameState->PlayerY = TruncateReal32ToInt32(newPlayerY);
	}

	DrawTimeMap(screenBuffer, currentTileMap);
	RenderPlayer(screenBuffer, gameState->PlayerX, gameState->PlayerY, playerWidth, playerHeight);
	FillAudioBuffer(thread, gameMemory, soundBuffer);
}

int32 IsPointEmpty(const real32& testX, const real32& testY, TileMap* tileMap)
{
	int32 isEmpty = 0;

	int32 tileX = TruncateReal32ToInt32((testX - tileMap->UpperLeftX) / tileMap->TileWidth);
	int32 tileY = TruncateReal32ToInt32((testY - tileMap->UpperLeftY) / tileMap->TileHeight);


	if ((tileX >= 0 && tileX < tileMap->CountX) && (tileY >= 0 && tileY < tileMap->CountY))
	{
		uint32 tileMapValue = GetTileVAlueUnchecked(tileMap, tileX, tileY);
		isEmpty = tileMapValue == 0;
	}

	return isEmpty;
}

int32 IsWorldPointEmpty(World* world, int32 tileMapX, int32 tileMapY, const real32& testX, const real32& testY)
{
	int32 isEmpty = 0;

	TileMap* tileMap = GetTileMap(world, tileMapX, tileMapY);

	if (tileMap)
	{
		int32 tileX = TruncateReal32ToInt32((testX - tileMap->UpperLeftX) / tileMap->TileWidth);
		int32 tileY = TruncateReal32ToInt32((testY - tileMap->UpperLeftY) / tileMap->TileHeight);

		if ((tileX >= 0 && tileX < tileMap->CountX) && (tileY >= 0 && tileY < tileMap->CountY))
		{
			uint32 tileMapValue = GetTileVAlueUnchecked(tileMap, tileX, tileY);
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

void DrawTimeMap(GameScreenBuffer* screenBuffer, TileMap* tileMap)
{
	for (size_t row = 0; row < tileMap->CountY; row++)
	{
		for (size_t column = 0; column < tileMap->CountX; column++)
		{
			Colour colour = {};

			(GetTileVAlueUnchecked(tileMap, column, row) == 1) ? colour = { 1.f, 1.f, 1.f } : colour = { 0.7f, 0.7f, 0.7f };

			real32 minX = tileMap->UpperLeftX + ((real32)column * tileMap->TileWidth);
			real32 maxX = minX + tileMap->TileWidth;

			real32 minY = tileMap->UpperLeftY + ((real32)row * tileMap->TileHeight);
			real32 maxY = minY + tileMap->TileHeight;

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

int32 RoundReal32ToInt32(real32 value)
{
	int32 result = (int32)(value + 0.5f);
	return result;
}

int32 TruncateReal32ToInt32(real32 value)
{
	int32 result = (int32)(value);
	return result;
}

inline int32 GetTileVAlueUnchecked(TileMap* tileMap, size_t tileX, size_t tileY)
{
	return tileMap->Tiles[tileY * tileMap->CountX + tileX];
}

inline TileMap* GetTileMap(World* world, size_t tileX, size_t tileY)
{
	TileMap* tileMap = 0;

	if ((tileX >= 0 && tileX <= world->CountX) && (tileY >= 0 && tileY <= world->CountY))
		tileMap = &world->TileMaps[tileY * world->CountX + tileX];

	return tileMap;
}