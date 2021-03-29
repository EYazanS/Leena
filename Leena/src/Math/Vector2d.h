#pragma once

struct Vector2d
{
	real32 X;
	real32 Y;

	inline real32& operator[](int32 index) { return (&X)[index]; }

	inline Vector2d &operator*=(real32 b);
	inline Vector2d &operator+=(Vector2d b);
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
	Vector2d result = {};

	result.X = a * b.X;
	result.Y = a * b.Y;

	return result;
}

inline Vector2d &Vector2d::operator+=(Vector2d b)
{
	*this = *this + b;
	return *this;
}

inline Vector2d &Vector2d::operator*=(real32 b)
{
	*this = *this * b;
	return *this;
}