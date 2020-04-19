#include "Leena.h"
#include <Windows.h>
#include <xaudio2.h>

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int XOffset, int YOffset);
void FillAudioBuffer(GameAudioBuffer& soundBuffer);
GameAudioBuffer ReadAudioBufferData(void* memory);

struct GameState
{
	int XOffset;
	int YOffset;
};

void GameUpdate(GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer& soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	if (!gameMemory->IsInitialized)
	{
		gameMemory->IsInitialized = true;
	}

	RenderWirdGradiend(screenBuffer, gameState->XOffset, gameState->YOffset);
	FillAudioBuffer(soundBuffer);

	for each (GameControllerInput controller in input->Controllers)
	{
		if (controller.IsAnalog)
		{
			gameState->XOffset += static_cast<int>(5 * controller.LeftStickAverageX);
			gameState->YOffset += static_cast<int>(5 * controller.LeftStickAverageY);
		}

		if (controller.MoveLeft.EndedDown)
			gameState->XOffset -= 5;

		if (controller.MoveRight.EndedDown)
			gameState->XOffset += 5;

		if (controller.MoveUp.EndedDown)
			gameState->YOffset += 5;

		if (controller.MoveDown.EndedDown)
			gameState->YOffset -= 5;
	}
}

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int XOffset, int YOffset)
{
	uint8* Row = (uint8*)gameScreenBuffer->Memory;

	for (int Y = 0; Y < gameScreenBuffer->Height; ++Y)
	{
		uint32* Pixel = (uint32*)Row;

		for (int X = 0; X < gameScreenBuffer->Width; ++X)
		{
			uint8 g = static_cast<uint8>(Y + YOffset);
			uint8 b = static_cast<uint8>(X + XOffset);

			/*
			Memory:   BB GG RR xx
			Register: xx RR GG BB
			Pixel (32-bits)
			*/

			*Pixel++ = (g << 8) | b;
		}

		Row += gameScreenBuffer->Pitch;
	}
}

void FillAudioBuffer(GameAudioBuffer& soundBuffer)
{
	if (!soundBuffer.BufferData)
	{
		DebugFileResult file = DebugPlatformReadEntireFile("src/resources/Water_Splash_SeaLion_Fienup_001.wav");

		auto result = ReadAudioBufferData(file.Memory);

		soundBuffer = result;
	}
}

GameAudioBuffer ReadAudioBufferData(void* memory)
{
	uint8* byte = (uint8*)memory; // Get the firs 4 bytes of the memory

	// Move to position 21
	byte += 20;

	uint16 formatTag = *((uint16*)byte);

	// Move to position 23
	byte += 2;

	uint16 channels = *((uint16*)byte);

	// Move to position 25
	byte += 2;

	uint32 samplesPerSec = *((uint32*)byte);

	// Move to position 29
	byte += 4;
	uint32 avgBytesPerSec = *((uint32*)byte);

	// Move to position 33
	byte += 4;
	uint16 blockAlign = *((uint16*)byte);

	// Move to position 35
	byte += 2;
	uint16 bitsPerSample = *((uint16*)byte);

	// Move to Data chunk

	char* characterToRead = ((char*)byte);

	while (*characterToRead != 'd' || *(characterToRead + 1) != 'a' || *(characterToRead + 2) != 't' || *(characterToRead + 3) != 'a')
	{
		characterToRead++;
	}
	
	characterToRead += 4;

	byte = (uint8*)(characterToRead);

	uint32 bufferSize = *((uint32*)byte);
	
	byte += 4;

	GameAudioBuffer result = { formatTag, channels, samplesPerSec, avgBytesPerSec, blockAlign, bitsPerSample, bufferSize, byte };

	return result;
}