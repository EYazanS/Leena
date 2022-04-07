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

struct R2
{
	union
	{
		struct
		{
			V2 Min;
			V2 Max;
		};

		V2 Elements[2];
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

inline V2 &operator+=(V2 &a, V2 b)
{
	a = a + b;
	return a;
}

inline V2 &operator*=(V2 &a, r32 b)
{
	a = a * b;

	return a;
}

inline r32 InnerProduct(V2 &a, V2 &b)
{
	r32 result = (a.X * b.X) + (a.Y * b.Y);

	return result;
}

inline r32 LengthSq(V2 v)
{
	r32 result = InnerProduct(v, v);

	return result;
}

inline r32 Length(V2 v)
{
	r32 result = SquareRoot(LengthSq(v));

	return result;
}

inline R2 RectMinMax(V2 min, V2 max)
{
	R2 rect = {min, max};

	return rect;
}

inline R2 RectMinDim(V2 min, V2 halfDim)
{
	R2 rect = {};

	rect.Min = min;
	rect.Max = min + halfDim;

	return rect;
}

inline R2 RectCenterHalfDim(V2 center, V2 halfDim)
{
	R2 rect = {};

	rect.Min = center - halfDim;
	rect.Max = center + halfDim;

	return rect;
}

inline R2 RectCenterDim(V2 center, V2 dim)
{
	R2 result = RectCenterHalfDim(center, 0.5f * dim);

	return result;
}

inline R2 AddRadiusToBound(R2 source, r32 radiusWidth, r32 radiusHeight)
{
	R2 result;

	result.Min = source.Min - V2{radiusWidth, radiusWidth};
	result.Max = source.Max + V2{radiusWidth, radiusWidth};

	return result;
}

inline b32 IsInRectangle(R2 rectangle, V2 test)
{
	b32 result = ((test.X >= rectangle.Min.X) &&
				  (test.Y >= rectangle.Min.Y) &&
				  (test.X < rectangle.Max.X) &&
				  (test.Y < rectangle.Max.Y));

	return (result);
}

inline V2 GetMinCorner(R2 rectangle)
{
	V2 result = rectangle.Min;

	return (result);
}

inline V2 GetMaxCorner(R2 rectangle)
{
	V2 result = rectangle.Max;

	return (result);
}

inline V2 GetCenterCorner(R2 rectangle)
{
	V2 result = 0.5f * (rectangle.Min + rectangle.Max);

	return (result);
}

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

		struct
		{
			r32 R;
			r32 G;
			r32 B;
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

inline V3 &operator+=(V3 &a, V3 b)
{
	a = a + b;
	return a;
}

inline V3 &operator*=(V3 &a, r32 b)
{
	a = a * b;
	return a;
}

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

inline V4 &operator+=(V4 &a, V4 b)
{
	a = a + b;
	return a;
}

inline V4 &operator*=(V4 &a, r32 b)
{
	a = a * b;
	return a;
}