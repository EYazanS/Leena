#if !defined(Leena_Intinsics_h)
#include <math.h>

// Math
inline int32 RoundReal32ToInt32(real32 value)
{
	int32 result = (int32)(value + 0.5f);
	return result;
}

inline int32 TruncateReal32ToInt32(real32 value)
{
	int32 result = (int32)(value);
	return result;
}

inline int32 FloorReal32ToInt32(real32 value)
{
	int32 result = (int32)floorf(value);
	return result;
}

inline real32 Sin(real32 angel)
{
	return sin(angel);
}

inline real32 Cos(real32 angel)
{
	return cos(angel);
}

inline real32 ATan2(real32 x, real32 y)
{
	return atan2(x, y);
}

#define Leena_Intinsics_h
#endif // !defined(Leena_Intinsics)