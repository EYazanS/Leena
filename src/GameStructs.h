#pragma once

enum class Key
{
	A,
	S,
	W,
	D,
	E,
	Q,
	R,
	Space,
	Esc,
	Ctrl,
	Alt,
	Enter,
	MoveUp,
	MoveRight,
	MoveDown,
	MoveLeft
};

struct GameSoundBuffer
{
	real32 Time; // In Seconds
	real32 SampleRate; // In HZ
	real32 Period; // In Seconds
	int32 SamplesPerSecond;
	uint32 SampleCount;
	void* BufferData;
};

struct GameScreenBuffer
{
	void* Memory;
	int Width;
	int Height;
	int Pitch;
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

	real32 LeftStickAverageX;
	real32 LeftStickAverageY;

	real32 RightStickAverageX;
	real32 RightStickAverageY;

	real32 RightTrigger;
	real32 LeftTrigger;

	union
	{
		GameButtonState Buttons[16];

		struct
		{
			GameButtonState A;
			GameButtonState X;
			GameButtonState Y;
			GameButtonState B;

			GameButtonState MoveUp;
			GameButtonState MoveDown;
			GameButtonState MoveRight;
			GameButtonState MoveLeft;

			GameButtonState DpadUp;
			GameButtonState DpadDown;
			GameButtonState DpadRight;
			GameButtonState DpadLeft;

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

struct GameClock
{
	real32 TimeElapsed;
};