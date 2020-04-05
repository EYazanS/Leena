#pragma once

#include <Windows.h>

struct Win32BitmapBuffer
{
	BITMAPINFO BitmapInfo;
	void* Memory;
	int BytesPerPixel;
	int Width;
	int Height;
	int Pitch;
};