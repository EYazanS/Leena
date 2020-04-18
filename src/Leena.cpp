#include "Leena.h"
#include <Windows.h>
#include <xaudio2.h>

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int XOffset, int YOffset);
void Win32FillSoundBuffer(GameSoundBuffer* soundBuffer);
RIFFData ReadRIFF(void* memory);
MTData ReadMT(void* memory);

struct GameState
{
	int XOffset;
	int YOffset;
};

void GameUpdate(GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameSoundBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	if (!gameMemory->IsInitialized)
	{
		gameMemory->IsInitialized = true;
	}

	RenderWirdGradiend(screenBuffer, gameState->XOffset, gameState->YOffset);
	Win32FillSoundBuffer(soundBuffer);

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

void Win32FillSoundBuffer(GameSoundBuffer* soundBuffer)
{
	if (!soundBuffer->BufferData)
	{

	}
}

RIFFData ReadRIFF(void* memory)
{
	int8* bytes4 = (int8*)memory;

	int8* subChunkId = bytes4++;
	int8* subChunkSize = bytes4++;
	int8* format = bytes4++;

	uint32* bytes = (uint32*)memory; // Get the firs 4 bytes of the memory

	RIFFData data
	{
		*bytes++,
		*bytes++,
		*bytes
	};

	return data;
}

MTData ReadMT(void* memory)
{
	uint32* bytes4 = (uint32*)memory; // Get the firs 4 bytes of the memory

	// Skip the RIFF data
	bytes4 += 4;

	uint32* subChunkId = bytes4++;
	uint32* subChunkSize = bytes4++;

	uint16* bytes2 = (uint16*)bytes4;

	uint16* audioFormat = bytes2++;
	uint16* channels = bytes2++;

	bytes4 = (uint32*)bytes2;

	uint32* sampleRate = bytes4++;
	uint32* byteRate = bytes4++;

	bytes2 = (uint16*)bytes4;

	uint16* blockAlign = bytes2++;
	uint16* bitsPerSample = bytes2++;

	MTData data
	{
		*subChunkId,
		*subChunkSize,
		*audioFormat,
		*channels,
		*sampleRate,
		*byteRate,
		*blockAlign,
		*bitsPerSample
	};

	return data;
}