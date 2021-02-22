#if !defined(Leena_Intinsics_h)
#include <math.h>

// Math
inline int32 RoundReal32ToInt32(real32 value)
{
	int32 result = (int32)roundf(value);
	return result;
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

#define Leena_Intinsics_h
#endif // !defined(Leena_Intinsics)