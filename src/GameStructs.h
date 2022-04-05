#pragma once

#ifdef __cplusplus
extern "C"
{
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
// TODO(casey): Moar compilerz!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif

#include <stdint.h>
#include <stddef.h>

	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;

	typedef size_t MemorySizeIndex;

	typedef int8_t i8;
	typedef int16_t i16;
	typedef int32_t i32;
	typedef int64_t i64;

	typedef i32 b32;

	typedef float r32;
	typedef double r64;

#define Pi32 3.14159265359f

#if Leena_Speed
#define Assert(Expression)
#define InvalidCodePath
#else
#define Assert(Expression) \
	if (!(Expression))     \
	{                      \
		*(int *)0 = 0;     \
	}
#define InvalidCodePath Assert(!"InvalidCodePath")
#endif // Leena_Speed

#define Killobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Killobytes(Value) * 1024LL)
#define Gigabytes(value) (Megabytes(value) * 1024LL)
#define Terabytes(value) (Gigabytes(value) * 1024LL)

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

	inline u32 SafeTruncateUInt64(u64 value)
	{
		// TODO(casey): Defines for maximum values
		Assert(value <= 0xFFFFFFFF);
		u32 result = (u32)value;
		return (result);
	}

	typedef struct ThreadContext
	{
		int PlaceHolder;
	} ThreadContext;
	
#if Leena_Internal
	struct DebugFileResult
	{
		void *Memory;
		u32 FileSize;
	};

#define Debug_Platform_Free_File_Memory(name) void name(ThreadContext *thread, void *memory)
	typedef Debug_Platform_Free_File_Memory(PlatformFreeFileMemory);

#define Debug_Platform_Read_Entire_File(name) DebugFileResult name(ThreadContext *thread, const char *fileName)
	typedef Debug_Platform_Read_Entire_File(PlatformReadEntireFile);

#define Debug_Platform_Write_Entire_File(name) b32 name(ThreadContext *thread, const char *fileName, u32 memorySize, void *memory)
	typedef Debug_Platform_Write_Entire_File(PlatformWriteEntireFile);
#endif

	struct ScreenBuffer
	{
		void *Memory;
		int Width;
		int Height;
		int Pitch;
		int BytesPerPixel;
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
		void *BufferData;
	};

	enum class KeyAction
	{
		ActionUp,
		ActionDown,
		ActionRight,
		ActionLeft,
		MoveUp,
		MoveDown,
		MoveRight,
		MoveLeft,
		Jump,
		Run,
		Start,
		Back,
		RS,
		LS,
		A,
		B,
		X,
		Y
	};

	struct ButtonState
	{
		b32 EndedDown;
		int HalfTransitionCount;
		KeyAction Action;
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
			ButtonState Buttons[12];

			struct
			{
				ButtonState DpadUp;
				ButtonState DpadDown;
				ButtonState DpadLeft;
				ButtonState DpadRight;
				ButtonState LeftShoulder;
				ButtonState RightShoulder;
				ButtonState A;
				ButtonState B;
				ButtonState X;
				ButtonState Y;
				ButtonState Start;
				ButtonState Select;

				// NOTE All buttons must be added above this line
				ButtonState Terminator;
			};
		};
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
				ButtonState Right;
				ButtonState Left;
				
				// NOTE All buttons must be added above this line
				ButtonState Terminator;
			};
		};
	};

	struct MouseInput
	{
		b32 IsConnected;

		u64 X;
		u64 Y;
	};

	struct KeyboardInput
	{
		b32 IsConnected;

		union
		{
			ButtonState Buttons[10];

			struct
			{
				ButtonState Up;
				ButtonState Down;
				ButtonState Left;
				ButtonState Right;
				ButtonState A;
				ButtonState B;
				ButtonState X;
				ButtonState Y;
				ButtonState Start;
				ButtonState Select;

				// NOTE All buttons must be added above this line
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
		void *PermanentStorage;
		void *TransiateStorage;
		b32 IsInitialized;

		PlatformFreeFileMemory *FreeFile;
		PlatformReadEntireFile *ReadFile;
		PlatformWriteEntireFile *WriteFile;
	};

	enum class KeyCode
	{
		Spacebar = 0x20,
		Enter = 0x0D,
		Shift = 0x10,
		LeftArrow = 0x25,
		UpArrow = 0x26,
		RightArrow = 0x27,
		DownArrow = 0x28,
		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4A,
		K = 0x4B,
		L = 0x4C,
		M = 0x4D,
		N = 0x4E,
		O = 0x4F,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5A,
	};

	struct WindowDimensions
	{
		i32 Width;
		i32 Height;
	};

	struct GameClock
	{
		r32 TimeElapsed;
	};

#ifdef __cplusplus
}
#endif