#include "Leena.h"

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int PlayerX, int PlayerY);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, uint64 playerX, uint64 playerY);
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);
int32 RoundReal32ToInt32(real32 value);
void DrawRectangle(
	GameScreenBuffer* gameScreenBuffer,
	real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY,
	Colour colour);
void DrawTimeMap(GameScreenBuffer* screenBuffer, uint32 tileMap[10][16]);

struct GameState
{
	int PlayerX;
	int PlayerY;
};

void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	if (!gameMemory->IsInitialized)
	{
		gameMemory->IsInitialized = true;
	}

	uint32 tileMap[9][16] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1
	};

	const int pixelsToMovePerSec = 100;

	real64 frameMovement = pixelsToMovePerSec * input->TimeToAdvance;

	if (input->Keyboard.A.EndedDown)
		gameState->PlayerX -= static_cast<int>(frameMovement);

	if (input->Keyboard.D.EndedDown)
		gameState->PlayerX += static_cast<int>(frameMovement);

	if (input->Keyboard.W.EndedDown)
		gameState->PlayerY -= static_cast<int>(frameMovement);

	if (input->Keyboard.S.EndedDown)
		gameState->PlayerY += static_cast<int>(frameMovement);

	if (gameState->PlayerX < 0)
		gameState->PlayerX = 0;

	if (gameState->PlayerY < 0)
		gameState->PlayerY = 0;

	if (gameState->PlayerX + 10 > screenBuffer->Width)
		gameState->PlayerX = screenBuffer->Width - 10;

	if (gameState->PlayerY + 10 > screenBuffer->Height)
		gameState->PlayerY = screenBuffer->Height - 10;

	for (GameControllerInput controller : input->Controllers)
	{
		if (controller.IsConnected && controller.IsAnalog)
		{
			gameState->PlayerX += static_cast<int>(frameMovement * controller.LeftStickAverageX);
			gameState->PlayerY += static_cast<int>(frameMovement * controller.LeftStickAverageY);
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

void DrawTimeMap(GameScreenBuffer* screenBuffer, uint32 tileMap[10][16])
{
	real32 tileHeight = screenBuffer->Height / 9.f;
	real32 tileWidth = screenBuffer->Width / 16.f;

	for (size_t y = 0; y < 9; y++)
	{
		for (size_t x = 0; x < 16; x++)
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

	uint32 colour = 0xFFFFFFFF;

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