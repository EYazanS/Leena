#pragma once

struct Vector2d
{
	union
	{
		struct
		{
			real32 X;
			real32 Y;
		};

		real32 Elements[2];
	};
};

inline Vector2d operator-(Vector2d a)
{
	Vector2d result = {};

	result.X = -a.X;
	result.Y = -a.Y;

	return result;
}

inline Vector2d operator+(Vector2d a, Vector2d b)
{
	Vector2d result = {};

	result.X = a.X + b.X;
	result.Y = a.Y + b.Y;

	return result;
}

inline Vector2d operator-(Vector2d a, Vector2d b)
{
	Vector2d result = {};

	result.X = a.X - b.X;
	result.Y = a.Y - b.Y;

	return result;
}

inline Vector2d operator*(Vector2d a, real32 b)
{
	Vector2d result = {};

	result.X = b * a.X;
	result.Y = b * a.Y;

	return result;
}

inline Vector2d operator*(real32 a, Vector2d b)
{
	return b * a;
}

inline Vector2d& operator+=(Vector2d& a, Vector2d b)
{
	a = a + b;
	return a;
}

inline Vector2d& operator*=(Vector2d& a, real32 b)
{
	a = a * b;

	return a;
}

inline real32 InnerProduct(Vector2d& a, Vector2d& b)
{
	real32 result = (a.X * b.X) + (a.Y * b.Y);

	return result;
}