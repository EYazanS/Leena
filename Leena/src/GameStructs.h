#pragma once

#if Leena_Internal
struct DebugFileResult
{
	void* Memory;
	uint32 FileSize;
};

#define Debug_Platform_Free_File_Memory(name) void name(void* memory)
typedef Debug_Platform_Free_File_Memory(PlatformFreeFileMemory);

#define Debug_Platform_Read_Entire_File(name) DebugFileResult name(const char* fileName)
typedef Debug_Platform_Read_Entire_File(PlatformReadEntireFile);

#define Debug_Platform_Write_Entire_File(name) bool32 name(const char* fileName, uint32 memorySize, void* memory)
typedef Debug_Platform_Write_Entire_File(PlatformWriteEntireFile);

#endif

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

struct GameAudioBuffer
{
	uint16 FormatTag;
	uint16 Channels;
	uint32 SamplesPerSec;
	uint32 AvgBytesPerSec;
	uint16 BlockAlign;
	uint16 BitsPerSample;
	uint32 BufferSize;
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

	PlatformFreeFileMemory* FreeFile;
	PlatformReadEntireFile* ReadFile;
	PlatformWriteEntireFile* WriteFile;
};

struct GameClock
{
	real32 TimeElapsed;
};