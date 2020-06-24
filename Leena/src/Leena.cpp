#include "Leena.h"

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int PlayerX, int PlayerY);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, uint64 playerX, uint64 playerY);
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);
int32 RoundReal32ToInt32(real32 value);
void DrawRectangle(GameScreenBuffer* gameScreenBuffer, real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY);

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

	for (GameControllerInput controller : input->Controllers)
	{
		if (controller.IsConnected && controller.IsAnalog)
		{
			gameState->PlayerX += static_cast<int>(frameMovement * controller.LeftStickAverageX);
			gameState->PlayerY += static_cast<int>(frameMovement * controller.LeftStickAverageY);
		}
	}

	RenderWirdGradiend(screenBuffer, 0, 0);
	RenderPlayer(screenBuffer, gameState->PlayerX, gameState->PlayerY);
	DrawRectangle(screenBuffer, gameState->PlayerX + 15.f, gameState->PlayerY + 15.f, gameState->PlayerX + 50.f, gameState->PlayerY + 50.f);
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

	if (playerX < 0)
		playerX = 0;

	if (playerY < 0)
		playerY = 0;

	if (playerX > gameScreenBuffer->Width)
		playerX = gameScreenBuffer->Width;

	if (playerY > gameScreenBuffer->Height)
		playerY = gameScreenBuffer->Height;

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

void DrawRectangle(GameScreenBuffer* gameScreenBuffer, real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY)
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

	uint8* endOfBuffer = ((uint8*)gameScreenBuffer->Memory) + ((uint64)gameScreenBuffer->Pitch * (uint64)gameScreenBuffer->Height);

	uint32 colour = 0xFFFFFFFF;

	uint8* row = (((uint8*)gameScreenBuffer->Memory) + minX * 4 + minY * gameScreenBuffer->Pitch);

	for (int32 y = minY; y < maxY; ++y)
	{
		uint32* pixel = (uint32*)row;
		for (int32 x = minX; x < maxX + 10; ++x)
		{
			*pixel++ = colour;
		}

		row += gameScreenBuffer->Pitch;
	}
}

int32 RoundReal32ToInt32(real32 value)
{
	int32 result = (int32)(value + 0.5f);
	return result;
}