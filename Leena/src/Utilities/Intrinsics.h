#if !defined(Leena_Intinsics_h)
#include <math.h>
#include <stdlib.h>

// Math
inline i32 SingOf(i32 value)
{
	i32 result = value >= 0 ? 1 : -1;

	return result;
}

inline i32 RoundReal32ToInt32(r32 value)
{
	i32 result = (i32)roundf(value);
	return result;
}

inline u32 RoundReal32ToUInt32(r32 val)
{
	u32 Result = (u32)roundf(val);
	return Result;
}

inline i32 TruncateReal32ToInt32(r32 value)
{
	i32 result = (i32)(value);
	return result;
}

inline u32 TruncateReal32ToUint32(r32 value)
{
	u32 result = (u32)(value);
	return result;
}

inline i32 FloorReal32ToInt32(r32 value)
{
	i32 result = (i32)floorf(value);
	return result;
}

inline i32 CeilReal32ToInt32(r32 value)
{
	i32 result = (i32)ceilf(value);
	return result;
}

inline r32 Sin(r32 angel)
{
	r32 result = (r32)sin(angel);

	return result;
}

inline r32 Cos(r32 angel)
{
	r32 result = (r32)cos(angel);

	return result;
}

inline r32 ATan2(r32 x, r32 y)
{
	r32 result = (r32)atan2(x, y);

	return result;
}

inline u32 AbsoluteInt32ToUInt32(i32 number)
{
	if (number < 0)
		number *= -1;

	return (u32)number;
}

inline u32 RotateLeft(u32 value, i32 amount)
{
#if COMPILER_MSVC
	u32 result = _rotl(value, amount);
#else
	amount &= 31;
	u32 result = (value << amount) | (value >> (32 - amount));
#endif
	return result;
}

inline u32 RotateRight(u32 value, i32 amount)
{
#if COMPILER_MSVC

	u32 result = _rotr(value, amount);
#else
	amount &= 31;
	u32 result = (value >> amount) | (value << (32 - amount));
#endif
	return result;
}

inline u32 Sqaure(i32 number)
{
	return number * number;
}

inline r32 Square(r32 value)
{
	r32 Result = value * value;

	return(Result);
}


inline r32 SqaureRoot(r32 number)
{
	r32 result = sqrtf(number);
	return result;
}

inline r32 Sqaure(r32 number)
{
	return number * number;
}

inline r64 Sqaure(r64 number)
{
	return number * number;
}

inline r64 AbsoluteValue(r32 number)
{
	r32 result = (r32)fabs(number);

	return result;
}

struct BitScanResult
{
	b32 Found;
	u32 Index;
};

inline BitScanResult FindLeastSigifigantSetBit(u32 value)
{
	BitScanResult result = {};
#if COMPILER_MSVC
	result.Found = _BitScanForward((unsigned long*)&result.Index, value);
#else
	for (u32 currentScanningIndex = 0; currentScanningIndex < 32; currentScanningIndex++)
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