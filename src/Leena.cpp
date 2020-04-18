#include "Leena.h"

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int XOffset, int YOffset);
void Win32FillSoundBuffer(GameSoundBuffer* soundBuffer);

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
		const float Pi = 3.1415f;

		soundBuffer->SampleCount = soundBuffer->SamplesPerSecond * 2;

		float* bufferData = new float[soundBuffer->SampleCount];

		for (uint32 i = 0; i < soundBuffer->SampleCount; i += 2)
		{
			*(bufferData + i) = sinf((soundBuffer->Time * 2 * Pi) / soundBuffer->Period);

			soundBuffer->Time += (1.0f / soundBuffer->SampleRate);             // move time forward one sample-tick

			if (soundBuffer->Time > soundBuffer->Period)
				soundBuffer->Time -= soundBuffer->Period;
		}

		soundBuffer->BufferData = bufferData;
	}
}