#pragma once

struct Vector3d
{
	union
	{
		struct
		{
			real32 X;
			real32 Y;
			real32 Z;
		};

		real32 Elements[3];
	};
};

inline Vector3d operator-(Vector3d a)
{
	Vector3d result = {};

	result.X = -a.X;
	result.Y = -a.Y;
	result.Z = -a.Z;

	return result;
}

inline Vector3d operator+(Vector3d a, Vector3d b)
{
	Vector3d result = {};

	result.X = a.X + b.X;
	result.Y = a.Y + b.Y;
	result.Z = a.Z + b.Z;

	return result;
}

inline Vector3d operator-(Vector3d a, Vector3d b)
{
	Vector3d result = {};

	result.X = a.X - b.X;
	result.Y = a.Y - b.Y;
	result.Z = a.Z - b.Z;

	return result;
}

inline Vector3d operator*(Vector3d a, real32 b)
{
	Vector3d result = {};

	result.X = b * a.X;
	result.Y = b * a.Y;
	result.Z = b * a.Z;

	return result;
}

inline Vector3d operator*(real32 a, Vector3d b)
{
	return b * a;
}

inline Vector3d& operator+=(Vector3d &a, Vector3d b)
{
	a = a + b;
	return a;
}

inline Vector3d& operator*=(Vector3d& a, real32 b)
{
	a = a * b;
	return a;
}