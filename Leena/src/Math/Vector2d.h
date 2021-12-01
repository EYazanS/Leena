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

inline r32 Length(V2 v)
{
	r32 result = SquareRoot(LengthSq(v));

	return result;
}

inline R2 RectMinMax(V2 min, V2 max)
{
	R2 rect = { min, max };

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

inline b32 IsInRectangle(R2 rectangle, V2 test)
{
	b32 result = ((test.X >= rectangle.Min.X) &&
		(test.Y >= rectangle.Min.Y) &&
		(test.X < rectangle.Max.X) &&
		(test.Y < rectangle.Max.Y));

	return(result);
}

inline V2 GetMinCorner(R2 rectangle)
{
	V2 result = rectangle.Min;

	return(result);
}

inline V2 GetMaxCorner(R2 rectangle)
{
	V2 result = rectangle.Max;

	return(result);
}

inline V2 GetCenterCorner(R2 rectangle)
{
	V2 result = 0.5f * (rectangle.Min + rectangle.Max);

	return(result);
}