#include <Windows.h>

#include "Defines.h"

LRESULT CALLBACK Win32WindowCallback(HWND, UINT, WPARAM, LPARAM);

Internal HWND Win32InitWindow(const HINSTANCE& instance, int cmdShow);
Internal void Win32ResizeDIBSection(int width, int height);
Internal void Win32UpdateWindow(HDC deviceContext, int width, int height);
Internal void RenderWirdGradiend(int XOffset, int YOffset);
Internal MSG Win32ProcessMessage(const HWND& windowHandle);

GlobalVariable bool IsRunning = true;

GlobalVariable BITMAPINFO BitmapInfo;
GlobalVariable 	HDC Devicecontext;
GlobalVariable 	int BytesPerPixel = 4;
GlobalVariable 	void* BitmapMemory;
GlobalVariable 	int BitmapWidth, BitmapHeight;

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance, PWSTR cmdLine, int cmdShow)
{
	HWND windowHandle = Win32InitWindow(instance, cmdShow);

	if (windowHandle)
	{
		int XOffset = 0;
		int YOffset = 0;

		while (IsRunning)
		{
			MSG msg = Win32ProcessMessage(windowHandle);
			RenderWirdGradiend(XOffset, YOffset);
			HDC deviceContext = GetDC(windowHandle);
			RECT clientRect;
			GetClientRect(windowHandle, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			Win32UpdateWindow(deviceContext, width, height);
			ReleaseDC(windowHandle, deviceContext);

			++XOffset;
		}
	}

	return 0;
}

Internal MSG Win32ProcessMessage(const HWND& windowHandle)
{
	MSG message;

	auto MessageResult = PeekMessage(&message, NULL, 0, 0, PM_REMOVE);

	if (MessageResult > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return message;
}

Internal HWND Win32InitWindow(const HINSTANCE& instance, int cmdShow)
{
	WNDCLASS window = {};

	const wchar_t className[] = L"Leena Game Engine";

	window.lpfnWndProc = Win32WindowCallback;
	window.hInstance = instance;
	window.lpszClassName = className;
	window.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&window);

	HWND windowHandle = CreateWindowEx(
		0,
		window.lpszClassName,
		L"Leena Game Engine",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,//DWORD dwStyle,
		CW_USEDEFAULT, //int X,
		CW_USEDEFAULT, //int Y,
		CW_USEDEFAULT, //int nWidth,
		CW_USEDEFAULT, //int nHeight,
		NULL,
		NULL,
		instance,
		NULL
	);

	if (windowHandle == NULL)
		return NULL;

	return windowHandle;
}

Internal void Win32UpdateWindow(HDC deviceContext, int width, int height)
{
	StretchDIBits(
		deviceContext,
		0, 0, BitmapWidth, BitmapHeight,
		0, 0, width, height,
		BitmapMemory,
		&BitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

Internal void Win32ResizeDIBSection(int width, int height)
{
	if (BitmapMemory)
		VirtualFree(BitmapMemory, NULL, MEM_RELEASE);

	BitmapWidth = width;
	BitmapHeight = height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);

	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;

	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = BitmapWidth * BitmapHeight * BytesPerPixel;

	BitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

Internal void RenderWirdGradiend(int XOffset, int YOffset)
{
	int Pitch = BitmapWidth * BytesPerPixel;
	uint8* Row = (uint8*)BitmapMemory;
	for (int Y = 0; Y < BitmapHeight; ++Y)
	{
		uint32* Pixel = (uint32*)Row;
		for (int X = 0; X < BitmapWidth; ++X)
		{
			uint8 g = (Y + YOffset);
			uint8 b = (X + XOffset);

			/*
			Memory:   BB GG RR xx
			Register: xx RR GG BB
			Pixel (32-bits)
			*/

			*Pixel++ = (g << 8) | b;
		}
		Row += Pitch;
	}
}

LRESULT CALLBACK Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_CLOSE:
		{
			IsRunning = false;
		} break;

		case WM_DESTROY:
		{
			IsRunning = false;
		} break;

		case WM_SIZE:
		{
			RECT rect;
			GetClientRect(windowHandle, &rect);
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			Win32ResizeDIBSection(width, height);
		} break;

		case WM_ACTIVATEAPP:
		{
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(windowHandle, &paint);
			RECT clientRect;
			GetClientRect(windowHandle, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			Win32UpdateWindow(deviceContext, width, height);
			EndPaint(windowHandle, &paint);
		} break;

		default:
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		} break;
	}

	return result;
}