#pragma once

struct V2
{
	union
	{
		struct
		{
			r32 X;
			r32 Y;
		};

		r32 Elements[2];
	};
};

inline V2 operator-(V2 a)
{
	V2 result = {};

	result.X = -a.X;
	result.Y = -a.Y;

	return result;
}

inline V2 operator+(V2 a, V2 b)
{
	V2 result = {};

	result.X = a.X + b.X;
	result.Y = a.Y + b.Y;

	return result;
}

inline V2 operator-(V2 a, V2 b)
{
	V2 result = {};

	result.X = a.X - b.X;
	result.Y = a.Y - b.Y;

	return result;
}

inline V2 operator*(V2 a, r32 b)
{
	V2 result = {};

	result.X = b * a.X;
	result.Y = b * a.Y;

	return result;
}

inline V2 operator*(r32 a, V2 b)
{
	return b * a;
}

inline V2& operator+=(V2& a, V2 b)
{
	a = a + b;
	return a;
}

inline V2& operator*=(V2& a, r32 b)
{
	a = a * b;

	return a;
}

inline r32 InnerProduct(V2& a, V2& b)
{
	r32 result = (a.X * b.X) + (a.Y * b.Y);

	return result;
}

inline r32 LengthSq(V2 v)
{
	r32 result = InnerProduct(v, v);

	return result;
}