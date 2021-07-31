#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	// TO Set the compilers if not set
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif


#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif // _MSC_VER

#endif

#if COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#endif // COMPILER_MSVC

	struct ThreadContext
	{
		int PlaceHolder;
	};

#if Leena_Internal
	struct DebugFileResult
	{
		void* Memory;
		u32 FileSize;
	};

#define Debug_Platform_Free_File_Memory(name) void name(ThreadContext* thread, void* memory)
	typedef Debug_Platform_Free_File_Memory(PlatformFreeFileMemory);

#define Debug_Platform_Read_Entire_File(name) DebugFileResult name(ThreadContext* thread, const char* fileName)
	typedef Debug_Platform_Read_Entire_File(PlatformReadEntireFile);

#define Debug_Platform_Write_Entire_File(name) b32 name(ThreadContext* thread, const char* fileName, u32 memorySize, void* memory)
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

	struct AudioBuffer
	{
		u16 FormatTag;
		u16 Channels;
		u32 SamplesPerSec;
		u32 AvgBytesPerSec;
		u16 BlockAlign;
		u16 BitsPerSample;
		u32 BufferSize;
		void* BufferData;
	};

	struct ScreenBuffer
	{
		void* Memory;
		int Width;
		int Height;
		int Pitch;
		int BytesPerPixel;
	};

	struct ButtonState
	{
		b32 EndedDown;
		int HalfTransitionCount;
	};

	struct WindowDimensions
	{
		i32 Width;
		i32 Height;
	};

	struct MouseInput
	{
		b32 IsConnected;

		u64 X;
		u64 Y;

		union
		{
			ButtonState Buttons[2];

			struct
			{
				ButtonState RightButton;
				ButtonState LeftButton;

				// Add button before this line so the assertion about the buttons array == the struct can hit properly 
				ButtonState Terminator;
			};
		};
	};

	struct ControllerInput
	{
		b32 IsConnected;
		b32 IsAnalog;

		r32 LeftStickAverageX;
		r32 LeftStickAverageY;

		r32 RightStickAverageX;
		r32 RightStickAverageY;

		r32 RightTrigger;
		r32 LeftTrigger;

		union
		{
			ButtonState Buttons[16];

			struct
			{
				ButtonState A;
				ButtonState X;
				ButtonState Y;
				ButtonState B;

				ButtonState MoveUp;
				ButtonState MoveDown;
				ButtonState MoveRight;
				ButtonState MoveLeft;

				ButtonState DpadUp;
				ButtonState DpadDown;
				ButtonState DpadRight;
				ButtonState DpadLeft;

				ButtonState LeftShoulder;
				ButtonState RightShoulder;

				ButtonState Start;
				ButtonState Back;

				// Add button before this line so the assertion about the buttons array == the struct can hit properly 
				ButtonState Terminator;
			};
		};
	};

	struct KeyboardInput
	{
		b32 IsConnected;

		union
		{
			ButtonState Buttons[16];

			struct
			{
				ButtonState A;
				ButtonState X;
				ButtonState Y;
				ButtonState B;

				ButtonState MoveUp;
				ButtonState MoveDown;
				ButtonState MoveRight;
				ButtonState MoveLeft;

				ButtonState DpadUp;
				ButtonState DpadDown;
				ButtonState DpadRight;
				ButtonState DpadLeft;

				ButtonState LeftShoulder;
				ButtonState RightShoulder;

				ButtonState Start;
				ButtonState Back;

				// Add button before this line so the assertion about the buttons array == the struct can hit properly 
				ButtonState Terminator;
			};
		};
	};

	struct GameInput
	{
		r64 TimeToAdvance;
		MouseInput Mouse;
		KeyboardInput Keyboard;
		ControllerInput Controller;
	};

	struct GameMemory
	{
		MemorySizeIndex PermanentStorageSize;
		MemorySizeIndex TransiateStorageSize;
		void* PermanentStorage;
		void* TransiateStorage;
		b32 IsInitialized;

		PlatformFreeFileMemory* FreeFile;
		PlatformReadEntireFile* ReadFile;
		PlatformWriteEntireFile* WriteFile;
	};

	struct GameClock
	{
		r32 TimeElapsed;
	};

#ifdef __cplusplus
}
#endif