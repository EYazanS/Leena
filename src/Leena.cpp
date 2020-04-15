#include "Leena.h"

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int XOffset, int YOffset);
void FillSoundBuffer(GameSoundBuffer* soundBuffer);

void GameUpdate(GameScreenBuffer* gameScreenBuffer, GameSoundBuffer* soundBuffer)
{
	int XOffset = 0;
	int YOffset = 0;

	RenderWirdGradiend(gameScreenBuffer, XOffset, YOffset);
	FillSoundBuffer(soundBuffer);
}

void RenderWirdGradiend(GameScreenBuffer* gameScreenBuffer, int XOffset, int YOffset)
{
	uint8* Row = (uint8*)gameScreenBuffer->Memory;

	for (int Y = 0; Y < gameScreenBuffer->Height; ++Y)
	{
		uint32* Pixel = (uint32*)Row;

		for (int X = 0; X < gameScreenBuffer->Width; ++X)
		{
			uint8 g = (Y + YOffset);
			uint8 b = (X + XOffset);

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

void FillSoundBuffer(GameSoundBuffer* soundBuffer)
{
	const int SAMPLE_RATE = 44100;
	const float PI = 3.1415f;
	const int NOTE_FREQ = 55;

	soundBuffer->VoiceBufferSampleCount = SAMPLE_RATE * 2;

	float* bufferData = new float[soundBuffer->VoiceBufferSampleCount];

	for (uint32 i = 0; i < soundBuffer->VoiceBufferSampleCount; i += 2)
	{
		*(bufferData + i) = sin(i * 2 * PI * NOTE_FREQ / SAMPLE_RATE);
		*(bufferData + i + 1) = sin(i * 2 * PI * (NOTE_FREQ + 2) / SAMPLE_RATE);
	}

	soundBuffer->BufferData = bufferData;
}