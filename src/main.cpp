#include <iostream>
#include <Windows.h>

LRESULT CALLBACK WindowCallback(HWND, UINT, WPARAM, LPARAM);
HWND* InitWindow(const HINSTANCE& instance, int cmdShow);
MSG ProcessMessage(const HWND& windowHandle);

static bool isRunning = true;

int WINAPI wWinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	PWSTR cmdLine,
	int cmdShow)
{
	auto windowHandle = InitWindow(instance, cmdShow);

	if (windowHandle)
	{
		while (isRunning)
		{
			auto msg = ProcessMessage(*windowHandle);
		}
	}

	return 0;
}

MSG ProcessMessage(const HWND& windowHandle)
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

HWND* InitWindow(const HINSTANCE& instance, int cmdShow)
{
	WNDCLASS window = {};

	const wchar_t className[] = L"Leena Game Engine";

	window.lpfnWndProc = WindowCallback;
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

LRESULT CALLBACK WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_CLOSE:
		{
			isRunning = false;
		} break;

		case WM_DESTROY:
		{
			isRunning = false;
		} break;

		case WM_SIZE:
		{
			OutputDebugString(L"RESIZE");
		} break;

		case WM_ACTIVATEAPP:
		{
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;

			HDC DeviceContext = BeginPaint(windowHandle, &Paint);

			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;

			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

			PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);

			EndPaint(windowHandle, &Paint);

		} break;

		default:
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		} break;
	}

	return result;
}