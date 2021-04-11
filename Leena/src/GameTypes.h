#pragma once
#include <cstdint>

/*
	Leena_Speed:
		0 - Slow build
		1 - Fast build

	Leena_Internal:
		0 - Public
		1 - Internal
*/

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t MemorySizeIndex;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef i32 b32;

typedef float  r32;
typedef double r64;

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

#define Killobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Killobytes(Value) * 1024LL)
#define Gigabytes(value) (Megabytes(value) * 1024LL)
#define Terabytes(value) (Gigabytes(value) * 1024LL)

#if Leena_Speed
#define Assert(Expression)
#else
#define Assert(Expression) \
	if (!(Expression)) { *(int*)0 = 0; }
#endif // Leena_Speed