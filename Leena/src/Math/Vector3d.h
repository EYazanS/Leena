#pragma once

struct V3
{
	union
	{
		struct
		{
			r32 X;
			r32 Y;
			r32 Z;
		};

		r32 Elements[3];
	};
};

inline V3 operator-(V3 a)
{
	V3 result = {};

	result.X = -a.X;
	result.Y = -a.Y;
	result.Z = -a.Z;

	return result;
}

inline V3 operator+(V3 a, V3 b)
{
	V3 result = {};

	result.X = a.X + b.X;
	result.Y = a.Y + b.Y;
	result.Z = a.Z + b.Z;

	return result;
}

inline V3 operator-(V3 a, V3 b)
{
	V3 result = {};

	result.X = a.X - b.X;
	result.Y = a.Y - b.Y;
	result.Z = a.Z - b.Z;

	return result;
}

inline V3 operator*(V3 a, r32 b)
{
	V3 result = {};

	result.X = b * a.X;
	result.Y = b * a.Y;
	result.Z = b * a.Z;

	return result;
}

inline V3 operator*(r32 a, V3 b)
{
	return b * a;
}

inline V3& operator+=(V3 &a, V3 b)
{
	a = a + b;
	return a;
}

inline V3& operator*=(V3& a, r32 b)
{
	a = a * b;
	return a;
}