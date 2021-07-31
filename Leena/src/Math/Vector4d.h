#pragma once

struct V4
{
	union
	{
		struct
		{
			r32 X;
			r32 Y;
			r32 Z;
			r32 W;
		};

		struct
		{
			r32 R;
			r32 G;
			r32 B;
			r32 A;
		};

		r32 Elements[4];
	};
};

inline V4 operator-(V4 a)
{
	V4 result = {};

	result.X = -a.X;
	result.Y = -a.Y;
	result.Z = -a.Z;
	result.W = -a.W;

	return result;
}

inline V4 operator+(V4 a, V4 b)
{
	V4 result = {};

	result.X = a.X + b.X;
	result.Y = a.Y + b.Y;
	result.Z = a.Z + b.Z;
	result.W = a.W + b.W;

	return result;
}

inline V4 operator-(V4 a, V4 b)
{
	V4 result = {};

	result.X = a.X - b.X;
	result.Y = a.Y - b.Y;
	result.Z = a.Z - b.Z;
	result.W = a.W - b.W;

	return result;
}

inline V4 operator*(V4 a, r32 b)
{
	V4 result = {};

	result.X = b * a.X;
	result.Y = b * a.Y;
	result.Z = b * a.Z;
	result.W = b * a.W;

	return result;
}

inline V4 operator*(r32 a, V4 b)
{
	return b * a;
}

inline V4& operator+=(V4& a, V4 b)
{
	a = a + b;
	return a;
}

inline V4& operator*=(V4& a, r32 b)
{
	a = a * b;
	return a;
}