#include "Leena.h"

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int PlayerX, int PlayerY);
void RenderPlayer(GameScreenBuffer* gameScreenBuffer, uint64 playerX, uint64 playerY);
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);

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
	if (!soundBuffer->BufferData)
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