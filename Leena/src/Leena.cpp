#include "Leena.h"

#define TileMapXCount 16
#define TileMapYCount 9

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int PlayerX, int PlayerY);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, uint64 playerX, uint64 playerY);
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);
inline int32 RoundReal32ToInt32(real32 value);
inline int32 TruncateReal32ToInt32(real32 value);
void DrawRectangle(
	GameScreenBuffer* gameScreenBuffer,
	real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY,
	Colour colour);
void DrawTimeMap(GameScreenBuffer* screenBuffer, uint32 tileMap[TileMapYCount][TileMapXCount]);

struct GameState
{
	int PlayerX;
	int PlayerY;
};

void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	uint32 tileMap[TileMapYCount][TileMapXCount] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1
	};

	real32 upperLeftX = 0;
	real32 upperLeftY = 0;
	real32 tileWidth = 60;
	real32 tileHeight = 60;

	if (!gameMemory->IsInitialized)
	{
		gameState->PlayerX = 140;
		gameState->PlayerY = 70;
		gameMemory->IsInitialized = true;
	}

	real32 pixelsToMovePerSec = 100.f;

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

	real32 newPlayerX = gameState->PlayerX + playerMovementX * (real32)input->TimeToAdvance;
	real32 newPlayerY = gameState->PlayerY + playerMovementY * (real32)input->TimeToAdvance;

	int32 playerTileX = TruncateReal32ToInt32((newPlayerX - upperLeftX) / tileWidth);
	int32 playerTileY = TruncateReal32ToInt32((newPlayerY - upperLeftY) / tileHeight);

	for (GameControllerInput controller : input->Controllers)
	{
		if (controller.IsConnected && controller.IsAnalog)
		{
			gameState->PlayerX += static_cast<int>(pixelsToMovePerSec * input->TimeToAdvance * controller.LeftStickAverageX);
			gameState->PlayerY += static_cast<int>(pixelsToMovePerSec * input->TimeToAdvance * controller.LeftStickAverageY);
		}
	}

	if ((playerTileX >= 0 && playerTileX < TileMapXCount) && (playerTileY >= 0 && playerTileY < TileMapYCount))
	{
		uint32 tileMapValue = tileMap[playerTileY][playerTileX];

		if (!tileMapValue)
		{
			gameState->PlayerX = TruncateReal32ToInt32(newPlayerX);
			gameState->PlayerY = TruncateReal32ToInt32(newPlayerY);
		}
	}

	DrawTimeMap(screenBuffer, tileMap);
	RenderPlayer(screenBuffer, gameState->PlayerX, gameState->PlayerY);
	FillAudioBuffer(thread, gameMemory, soundBuffer);
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

void DrawTimeMap(GameScreenBuffer* screenBuffer, uint32 tileMap[TileMapYCount][TileMapXCount])
{
	real32 tileHeight = 60;
	real32 tileWidth = 60;

	for (size_t y = 0; y < TileMapYCount; y++)
	{
		for (size_t x = 0; x < TileMapXCount; x++)
		{
			Colour colour = {};

			(tileMap[y][x] == 1)
				? colour = { 1.f, 1.f, 1.f }
			: colour = { 0.7f, 0.7f, 0.7f };

			DrawRectangle(screenBuffer, tileWidth * x, tileHeight * y, (tileWidth * x) + tileWidth, (tileHeight * y) + tileHeight, colour);
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

void RenderPlayer(GameScreenBuffer* gameScreenBuffer, uint64 playerX, uint64 playerY)
{
	uint8* endOfBuffer = ((uint8*)gameScreenBuffer->Memory) + ((uint64)gameScreenBuffer->Pitch * (uint64)gameScreenBuffer->Height);

	uint32 colour = 0xFFF00FFF;

	int64 top = playerY;

	uint64 bottom = playerY + 10;

	for (uint64 x = playerX; x < playerX + 10; ++x)
	{
		uint8* pixel = (((uint8*)gameScreenBuffer->Memory) + x * 4 + top * gameScreenBuffer->Pitch);

		for (uint64 y = top; y < bottom; ++y)
		{
			if (pixel >= gameScreenBuffer->Memory && ((pixel + 4) < endOfBuffer))
			{
				*(uint32*)pixel = colour;
				pixel += gameScreenBuffer->Pitch;
			}
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

	uint8* row = (((uint8*)gameScreenBuffer->Memory) + minX * 4 + minY * gameScreenBuffer->Pitch);

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