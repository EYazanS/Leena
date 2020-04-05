#include <Windows.h>
#include "Defines.h"

LRESULT CALLBACK Win32WindowCallback(HWND, UINT, WPARAM, LPARAM);
Internal HWND* Win32InitWindow(const HINSTANCE& instance, int cmdShow);
Internal void Win32ResizeDIBSection(int width, int height);
MSG Win32ProcessMessage(const HWND& windowHandle);

GlobalVariable bool IsRunning = true;
GlobalVariable BITMAPINFO BitmapInfo;
GlobalVariable 	void* Buffer;
GlobalVariable 	HBITMAP BitmapHandle;
GlobalVariable 	HDC Devicecontext;

int WINAPI wWinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	PWSTR cmdLine,
	int cmdShow)
{
	auto windowHandle = Win32InitWindow(instance, cmdShow);

	if (windowHandle)
	{
		while (IsRunning)
		{
			auto msg = Win32ProcessMessage(*windowHandle);
		}
	}

	return 0;
}

MSG Win32ProcessMessage(const HWND& windowHandle)
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

Internal HWND* Win32InitWindow(const HINSTANCE& instance, int cmdShow)
{
	WNDCLASS window = {};

	const wchar_t className[] = L"Leena Game Engine";

	window.lpfnWndProc = Win32WindowCallback;
	window.hInstance = instance;
	window.lpszClassName = className;
	window.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&window);

	auto windowHandle = CreateWindowEx(
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

	return &windowHandle;
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
			RECT rect = {};
			GetClientRect(windowHandle, &rect);

			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;

			Win32ResizeDIBSection(width, height);

			OutputDebugString(L"RESIZE");
		} break;

		case WM_ACTIVATEAPP:
		{
		} break;

		case WM_PAINT:
		{
			RECT rect = {};
			GetClientRect(windowHandle, &rect);

			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;

			Win32ResizeDIBSection(width, height);

		} break;

		default:
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		} break;
	}

	return result;
}

Internal void Win32UpdateWindow(HWND windowHandle, int x, int y, int width, int height)
{
	PAINTSTRUCT paint;

	auto hdc = BeginPaint(windowHandle, &paint);

	StretchDIBits(
		hdc,
		x, y, width, height,
		x, y, width, height,
		Buffer,
		&BitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

Internal void Win32ResizeDIBSection(int width, int height)
{
	if (BitmapHandle)
		DeleteObject(BitmapHandle);

	if (!Devicecontext)
		Devicecontext = CreateCompatibleDC(0);

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);

	BitmapInfo.bmiHeader.biWidth = width;
	BitmapInfo.bmiHeader.biHeight = height;
	BitmapInfo.bmiHeader.biPlanes = 1;

	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	CreateDIBSection(Devicecontext, &BitmapInfo, DIB_RGB_COLORS, &Buffer, NULL, NULL);
}