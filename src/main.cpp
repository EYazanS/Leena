#include <iostream>
#include <Windows.h>

LRESULT WindowCallback(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	PWSTR cmdLine,
	int cmdShow)
{
	WNDCLASS window = {};

	const wchar_t className[] = L"Leena Game Engine";

	window.lpfnWndProc = WindowCallback;
	window.hInstance = instance;
	window.lpszClassName = className;

	RegisterClass(&window);

	auto windowHandle = CreateWindowEx(
		0,
		className,
		"Leena Game Engine",
		WS_OVERLAPPEDWINDOW,
		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		instance,
		NULL);

	if (windowHandle == NULL)
	{
		return 0;
	}
	
	return 0;
}

LRESULT WindowCallback(HWND window, UINT, WPARAM, LPARAM)
{
	return 0;
}