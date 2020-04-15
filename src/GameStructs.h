#pragma once

#include <Windows.h>
#include "Defines.h"

struct Win32BitmapBuffer
{
	BITMAPINFO Info;
	void* Memory;
	int BytesPerPixel;
	int Width;
	int Height;
	int Pitch;
};

struct GameSoundBuffer
{
	uint32 VoiceBufferSampleCount;
	int SampleRate;
	int Frequency;
	real32 WavePeriod;
	real32 Time;
	real32 BufferData[96000];
};

struct GameButtonState
{
	bool32 EndedDown;
	int HalfTransitionCount;
};

struct GameControllerInput
{
	bool32 IsConnected;
	bool32 IsAnalog;
	real32 StickAverageX;
	real32 StickAverageY;

	union
	{
		GameButtonState Buttons[12];

		struct
		{
			GameButtonState ActionUp;
			GameButtonState ActionDown;
			GameButtonState ActionRight;
			GameButtonState ActionLeft;

			GameButtonState MoveUp;
			GameButtonState MoveDown;
			GameButtonState MoveRight;
			GameButtonState MoveLeft;

			GameButtonState LeftShoulder;
			GameButtonState RightShoulder;

			GameButtonState Start;
			GameButtonState Back;

			// Add button before this line so the assertion about the buttons array == the struct can hit properly 
			GameButtonState Terminator;
		};
	};
};

struct GameInput
{
	GameControllerInput Controllers[5];
};

struct GameMemory
{
	uint64 PermenantStorageSize;
	uint64 TransiateStorageSize;
	void* PermenantStorage;
	void* TransiateStorage;
	bool32 IsInitialized;
};