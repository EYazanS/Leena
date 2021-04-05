#if !defined(Leena_Intinsics_h)
#include <math.h>
#include <stdlib.h>

// Math
inline int32 RoundReal32ToInt32(real32 value)
{
	int32 result = (int32)roundf(value);
	return result;
}

inline uint32 RoundReal32ToUInt32(real32 val)
{
	uint32 Result = (uint32)roundf(val);
	return Result;
}

inline int32 TruncateReal32ToInt32(real32 value)
{
	int32 result = (int32)(value);
	return result;
}

inline uint32 TruncateReal32ToUint32(real32 value)
{
	uint32 result = (uint32)(value);
	return result;
}

inline int32 FloorReal32ToInt32(real32 value)
{
	int32 result = (int32)floorf(value);
	return result;
}

inline real32 Sin(real32 angel)
{
	real32 result = (real32)sin(angel);

	return result;
}

inline real32 Cos(real32 angel)
{
	real32 result = (real32)cos(angel);

	return result;
}

inline real32 ATan2(real32 x, real32 y)
{
	real32 result = (real32)atan2(x, y);

	return result;
}

inline uint32 AbsoluteInt32ToUInt32(int32 number)
{
	if (number < 0)
		number *= -1;

	return (uint32)number;
}

inline uint32 RotateLeft(uint32 value, int32 amount)
{
	uint32 result = _rotl(value, amount);

	return result;
}

inline uint32 RotateRight(uint32 value, int32 amount)
{
	uint32 result = _rotr(value, amount);

	return result;
}

inline uint32 Sqaure(int32 number)
{
	return number * number;
}

inline real32 SqaureRoot(real32 number)
{
	real32 result = sqrtf(number);
	return result;
}

inline real32 Sqaure(real32 number)
{
	return number * number;
}

inline real64 Sqaure(real64 number)
{
	return number * number;
}

inline real64 AbsoluteValue(real32 number)
{
	real32 result = (real32)fabs(number);

	return result;
}

struct BitScanResult
{
	bool32 Found;
	uint32 Index;
};

inline BitScanResult FindLeastSigifigantSetBit(uint32 value)
{
	BitScanResult result = {};
#if COMPILER_MSVC
	result.Found = _BitScanForward((unsigned long*)&result.Index, value);
#else
	for (uint32 currentScanningIndex = 0; currentScanningIndex < 32; currentScanningIndex++)
	{
		if (value & 1 << currentScanningIndex)
		{
			result.Found = true;
			result.Index = currentScanningIndex;
			break;
		}
	}
#endif
	return result;
}


#if defined(COMPILER_MSVC)
//  Microsoft 
#define DllExport extern "C" __declspec(dllexport)
#define DllImport __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define DllExport extern "C" __attribute__((visibility("default")))
#define DllIMPORT
#endif

#define Leena_Intinsics_h

#endif // !defined(Leena_Intinsics)