#include "Leena.h"
#include <Windows.h>
#include <xaudio2.h>

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int XOffset, int YOffset);
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);

struct GameState
{
	int XOffset;
	int YOffset;
};

void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	if (!gameMemory->IsInitialized)
	{
		gameMemory->IsInitialized = true;
	}

	RenderWirdGradiend(screenBuffer, gameState->XOffset, gameState->YOffset);
	FillAudioBuffer(thread, gameMemory, soundBuffer);

	if (input->Keyboard.A.EndedDown)
		gameState->XOffset -= 5;

	if (input->Keyboard.D.EndedDown)
		gameState->XOffset += 5;

	if (input->Keyboard.W.EndedDown)
		gameState->YOffset += 5;

	if (input->Keyboard.S.EndedDown)
		gameState->YOffset -= 5;

	if (input->Mouse.LeftButton.EndedDown)
	{
		gameState->XOffset += static_cast<int>(input->Mouse.X / 5);
		gameState->YOffset += static_cast<int>(input->Mouse.Y / 5);
	}

	for each (GameControllerInput controller in input->Controllers)
	{
		if (controller.IsConnected)
		{
			if (controller.IsAnalog)
			{
				gameState->XOffset += static_cast<int>(5 * controller.LeftStickAverageX);
				gameState->YOffset += static_cast<int>(5 * controller.LeftStickAverageY);
			}
		}
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