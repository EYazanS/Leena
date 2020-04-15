#pragma once
#include <cstdint>

#define LocalPersist static
#define GlobalVariable static

#define internal static

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef float  real32;
typedef double real64;

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

#define Killobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Killobytes(Value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)
#define Assert(Expression) \
	if (!(Expression)) { *(int*)0 = 0; }