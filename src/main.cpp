#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <tuple>

#include "Defines.h"
#include "GameStructs.h"

LRESULT CALLBACK Win32WindowCallback(HWND, UINT, WPARAM, LPARAM);

Internal HWND Win32InitWindow(const HINSTANCE& instance, int cmdShow);
Internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int width, int height);
Internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, int width, int height);
Internal void RenderWirdGradiend(Win32BitmapBuffer* bitmapBuffer, int XOffset, int YOffset);
Internal MSG Win32ProcessMessage(const HWND& windowHandle);
Internal std::tuple<int, int> GetWindowDimensions(HWND windowHandle);
Internal HRESULT Wind32InitializeXAudio(int SampleBits);
Internal void PlayTestSound();
Internal HRESULT FillSoundBuffer(int SampleBits);

GlobalVariable bool IsRunning = true;
GlobalVariable bool shouldPlaySound = false;
GlobalVariable Win32BitmapBuffer GlobalBitmapBuffer;
GlobalVariable IXAudio2* xAudio;
GlobalVariable IXAudio2SourceVoice* sourceVoice;
GlobalVariable XAUDIO2_BUFFER buffer = { 0 };

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance, PWSTR cmdLine, int cmdShow)
{
	HWND windowHandle = Win32InitWindow(instance, cmdShow);

	if (windowHandle)
	{
		const int SampleBits = 32;

		// Get the performance frequence
		LARGE_INTEGER performanceFrequenceResult;
		QueryPerformanceFrequency(&performanceFrequenceResult);
		int64 performanceFrequence = performanceFrequenceResult.QuadPart;

		Win32ResizeDIBSection(&GlobalBitmapBuffer, 1280, 720);
		Wind32InitializeXAudio(SampleBits);

		int XOffset = 0;
		int YOffset = 0;

		FillSoundBuffer(32);

		LARGE_INTEGER lastCounter;
		QueryPerformanceCounter(&lastCounter);
		while (IsRunning)
		{
			MSG msg = Win32ProcessMessage(windowHandle);

			for (uint8 controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
			{
				XINPUT_STATE controllerState;

				if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
				{
					// Controller is connected
					auto* pad = &controllerState.Gamepad;

					bool dPadUp = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
					bool dPadDown = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
					bool dPadRight = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
					bool dPadleft = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;

					bool start = pad->wButtons & XINPUT_GAMEPAD_START;
					bool back = pad->wButtons & XINPUT_GAMEPAD_BACK;

					bool rightShould = pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
					bool leftShoulder = pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;

					bool rightThumb = pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
					bool leftThumb = pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB;

					bool aButton = pad->wButtons & XINPUT_GAMEPAD_A;
					bool bButton = pad->wButtons & XINPUT_GAMEPAD_B;
					bool xButton = pad->wButtons & XINPUT_GAMEPAD_X;
					bool yButton = pad->wButtons & XINPUT_GAMEPAD_Y;

					int16 leftStickX = pad->sThumbLX;
					int16 leftStickY = pad->sThumbLY;

					XINPUT_VIBRATION vibrations = {};

					if (aButton)
					{
						YOffset += 2;
						vibrations.wLeftMotorSpeed = 60000;
						vibrations.wRightMotorSpeed = 60000;

						PlayTestSound();
					}

					XInputSetState(controllerIndex, &vibrations);
				}
				else
				{
					// Controller is not connected

				}
			}

			RenderWirdGradiend(&GlobalBitmapBuffer, XOffset, YOffset);
			HDC deviceContext = GetDC(windowHandle);
			auto [width, height] = GetWindowDimensions(windowHandle);
			Win32DisplayBufferInWindow(&GlobalBitmapBuffer, deviceContext, width, height);
			ReleaseDC(windowHandle, deviceContext);
			++XOffset;

			if (sourceVoice && !shouldPlaySound)
			{
				sourceVoice->Stop();
			}

			// Display performance counter
			LARGE_INTEGER endCounter;
			QueryPerformanceCounter(&endCounter);

			int64 counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;

			int64 msPerFrame = 1000 * counterElapsed / performanceFrequence;
			int64 fps = performanceFrequence / counterElapsed;

			// Register last counter we got
			char formatBuffer[40];
			wsprintfA(formatBuffer, "ms/frame: %I64u - %I64ufps\n", msPerFrame, fps);
			OutputDebugStringA(formatBuffer);
			
			lastCounter = endCounter;
		}
	}

	return 0;
}

Internal std::tuple<int, int> GetWindowDimensions(HWND windowHandle)
{
	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	return std::make_tuple(width, height);
}

Internal MSG Win32ProcessMessage(const HWND& windowHandle)
{
	MSG message;

	BOOL MessageResult = PeekMessage(&message, NULL, 0, 0, PM_REMOVE);

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

Internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, int width, int height)
{
	// TOOD: aspect ratio correction 
	StretchDIBits(
		deviceContext,
		// Window Size - Destination
		0, 0, width, height,
		// Buffer Size - Source 
		0, 0, bitmapBuffer->Width, bitmapBuffer->Height,
		bitmapBuffer->Memory,
		&bitmapBuffer->Info,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

Internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int width, int height)
{
	if (bitmapBuffer->Memory)
		VirtualFree(bitmapBuffer->Memory, NULL, MEM_RELEASE);

	bitmapBuffer->Width = width;
	bitmapBuffer->Height = height;
	bitmapBuffer->BytesPerPixel = 4;
	bitmapBuffer->Pitch = bitmapBuffer->Width * bitmapBuffer->BytesPerPixel;

	// If the height is negative, it goes top down, instead of down up
	bitmapBuffer->Info.bmiHeader.biSize = sizeof(bitmapBuffer->Info.bmiHeader);
	bitmapBuffer->Info.bmiHeader.biWidth = bitmapBuffer->Width;
	bitmapBuffer->Info.bmiHeader.biHeight = -bitmapBuffer->Height;
	bitmapBuffer->Info.bmiHeader.biPlanes = 1;
	bitmapBuffer->Info.bmiHeader.biBitCount = 32;
	bitmapBuffer->Info.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = bitmapBuffer->Width * bitmapBuffer->Height * bitmapBuffer->BytesPerPixel;

	bitmapBuffer->Memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

Internal void RenderWirdGradiend(Win32BitmapBuffer* bitmapBuffer, int XOffset, int YOffset)
{
	uint8* Row = (uint8*)bitmapBuffer->Memory;

	for (int Y = 0; Y < bitmapBuffer->Height; ++Y)
	{
		uint32* Pixel = (uint32*)Row;

		for (int X = 0; X < bitmapBuffer->Width; ++X)
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

		Row += bitmapBuffer->Pitch;
	}
}

Internal HRESULT Wind32InitializeXAudio(int SampleBits)
{
	// TODO: UncoInitialize on error.
	HRESULT result;

	if (FAILED(result = CoInitialize(NULL)))
		return result;

	if (FAILED(result = XAudio2Create(&xAudio)))
		return result;

	IXAudio2MasteringVoice* masteringVoice;

	if (FAILED(result = xAudio->CreateMasteringVoice(&masteringVoice)))
		return result;

	const int CHANNELS = 2;
	const int SAMPLE_RATE = 44100;

	WAVEFORMATEX waveFormat = { 0 };
	waveFormat.wBitsPerSample = SampleBits;
	waveFormat.nAvgBytesPerSec = (SampleBits / 8) * CHANNELS * SAMPLE_RATE;
	waveFormat.nChannels = CHANNELS;
	waveFormat.nBlockAlign = CHANNELS * (SampleBits / 8);
	waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	waveFormat.nSamplesPerSec = SAMPLE_RATE;

	if (FAILED(result = xAudio->CreateSourceVoice(&sourceVoice, &waveFormat)))
		return result;
}

Internal void PlayTestSound()
{
	sourceVoice->Start();
}

Internal HRESULT FillSoundBuffer(int SampleBits)
{
	HRESULT result = {};

	const int SAMPLE_RATE = 44100;
	const float PI = 3.1415;
	const int VOICE_BUFFER_SAMPLE_COUNT = SAMPLE_RATE * 2;
	const int NOTE_FREQ = 55;

	float* bufferData = new float[VOICE_BUFFER_SAMPLE_COUNT];

	for (int i = 0; i < VOICE_BUFFER_SAMPLE_COUNT; i += 2)
	{
		*(bufferData + i) = sin(i * 2 * PI * NOTE_FREQ / SAMPLE_RATE);
		*(bufferData + i + 1) = sin(i * 2 * PI * (NOTE_FREQ + 2) / SAMPLE_RATE);
	}

	buffer.pAudioData = (BYTE*)bufferData;
	buffer.AudioBytes = VOICE_BUFFER_SAMPLE_COUNT * (SampleBits / 8);
	buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

	if (FAILED(sourceVoice->SubmitSourceBuffer(&buffer)))
		return result;
}

Internal real32 Win32ProcessXInputStickValues(real32 value, int16 deadZoneThreshold)
{
	real32 result = 0.f;

	if (value < -deadZoneThreshold)
		result = (real32)(value + deadZoneThreshold) / (32768.f - deadZoneThreshold);
	else if (value > deadZoneThreshold)
		result = (real32)(value + deadZoneThreshold) / (32767.f - deadZoneThreshold);

	return result;
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
		} break;

		case WM_ACTIVATEAPP:
		{
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 vkCode = wParam;

			bool wasDown = ((lParam & (1 << 30)) != 0);
			bool isDown = ((lParam & (1 << 31)) == 0);

			switch (vkCode)
			{
				case 'W':
				{
					if (isDown)
					{
						PlayTestSound();
						shouldPlaySound = true;
					}
					else
					{
						shouldPlaySound = false;
					}
				} break;

				case 'A':
				{

				} break;

				case 'S':
				{

				} break;

				case 'D':
				{

				} break;

				case 'E':
				{

				} break;

				case 'Q':
				{

				} break;

				case VK_UP:
				{

				} break;

				case VK_DOWN:
				{

				} break;

				case VK_RIGHT:
				{

				} break;

				case VK_LEFT:
				{

				} break;

				case VK_SPACE:
				{

				} break;

				case VK_ESCAPE:
				{

				} break;

				case VK_F4:
				{
					// Is alt button held down
					if ((lParam & (1 << 29)) != 0)
						IsRunning = false;

				} break;

				default:
				{

				} break;
			}
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(windowHandle, &paint);
			auto [width, height] = GetWindowDimensions(windowHandle);
			Win32DisplayBufferInWindow(&GlobalBitmapBuffer, deviceContext, width, height);
			EndPaint(windowHandle, &paint);
		} break;

		default:
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		} break;
	}

	return result;
}