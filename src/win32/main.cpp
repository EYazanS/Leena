#include <Leena.h>

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <tuple>

struct ProgramState
{
	bool IsRunning;
};

struct Win32BitmapBuffer
{
	BITMAPINFO Info;
	void* Memory;
	int BytesPerPixel;
	int Width;
	int Height;
	int Pitch;
};

struct Wind32SoundBuffer
{
	int32 SampleBits;
	int32 SamplesPerSecond;
	uint8 Channels;
	IXAudio2SourceVoice* SourceVoice;
};

#define LocalPersist static
#define GlobalVariable static
#define internal static

// Windows
LRESULT CALLBACK Win32WindowCallback(HWND, UINT, WPARAM, LPARAM);
internal inline ProgramState* GetAppState(HWND handle);
internal HWND Win32InitWindow(const HINSTANCE& instance, int cmdShow, ProgramState* state);
internal MSG Win32ProcessMessage(const HWND& windowHandle);
internal int64 GetPerformanceFrequence();
internal int64 QueryPerformance();

// Audio
internal HRESULT Wind32InitializeXAudio(IXAudio2* xAudio, Wind32SoundBuffer* soundBuffer);
internal HRESULT FillSoundBuffer(IXAudio2SourceVoice* sourceVoice, GameSoundBuffer* soundBuffer);
internal void PlayGameSound(IXAudio2SourceVoice* sourceVoice);

// Input

// Graphics
internal std::tuple<int, int> GetWindowDimensions(HWND windowHandle);
internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int width, int height);
internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, int width, int height);
internal void DrawBuffer(const HWND& windowHandle);

GlobalVariable Win32BitmapBuffer GlobalBitmapBuffer;

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance, PWSTR cmdLine, int showCommand)
{
	ProgramState programState = { };

	HWND windowHandle = Win32InitWindow(instance, showCommand, &programState);

	if (windowHandle)
	{
		// Get the performance frequence
		int64 performanceFrequence = GetPerformanceFrequence();

		programState.IsRunning = true;

		Win32ResizeDIBSection(&GlobalBitmapBuffer, 1280, 720);

		IXAudio2* xAudio = {};

		Wind32SoundBuffer soundBuffer = {};

		soundBuffer.SamplesPerSecond = 44100; // Equals 44.1 kHz for pcm 
		soundBuffer.Channels = 2;
		soundBuffer.SampleBits = 16;

		Wind32InitializeXAudio(xAudio, &soundBuffer);

		int64 lastCounter = QueryPerformance();

		// Get how many cycle the cpu went through
		uint64 lastCycleCount = __rdtsc();

		// Uncomment when we have proper wave to play ...
		// PlayGameSound(soundBuffer.SourceVoice);
		
		while (programState.IsRunning)
		{
			Win32ProcessMessage(windowHandle);

			for (uint8 controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
			{
				XINPUT_STATE controllerState;

				// If the controller is connected
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
						vibrations.wLeftMotorSpeed = 60000;
						vibrations.wRightMotorSpeed = 60000;
					}

					XInputSetState(controllerIndex, &vibrations);
				}
			}

			GameScreenBuffer screenBuffer =
			{
				GlobalBitmapBuffer.Memory,
				GlobalBitmapBuffer.Width,
				GlobalBitmapBuffer.Height,
				GlobalBitmapBuffer.Pitch,
			};

			GameSoundBuffer gameSoundBuffer = {};

			gameSoundBuffer.SamplesPerSecond = 48000;
			gameSoundBuffer.SampleRate = 44.1f; // in HZ
			gameSoundBuffer.Time = 1.f;; // in Seconds
			gameSoundBuffer.Period = 0.3f; // in Seconds, 1/3 of a second

			GameUpdate(&screenBuffer, &gameSoundBuffer);

			FillSoundBuffer(soundBuffer.SourceVoice, &gameSoundBuffer);
			DrawBuffer(windowHandle);

			// Display performance counter
			uint64 endCycleCount = __rdtsc();

			int64 endCounter = QueryPerformance();

			int64 counterElapsed = endCounter - lastCounter;
			int64 cyclesElapsed = endCycleCount - lastCycleCount;

			// how many cycles the cpu went throught a single frame
			int64 megaCyclesPerframe = cyclesElapsed / (1000 * 1000);

			int64 msPerFrame = 1000 * counterElapsed / performanceFrequence;
			int64 fps = performanceFrequence / counterElapsed;

			// Register last counter we got
			char formatBuffer[256];
			wsprintfA(formatBuffer, "%I64ums/f - %I64ufps - %I64uc/f\n", msPerFrame, fps, megaCyclesPerframe);
			OutputDebugStringA(formatBuffer);

			lastCycleCount = endCycleCount;
			lastCounter = endCounter;
		}
	}

	return 0;
}

internal void DrawBuffer(const HWND& windowHandle)
{
	HDC deviceContext = GetDC(windowHandle);
	auto [width, height] = GetWindowDimensions(windowHandle);
	Win32DisplayBufferInWindow(&GlobalBitmapBuffer, deviceContext, width, height);
	ReleaseDC(windowHandle, deviceContext);
}

internal std::tuple<int, int> GetWindowDimensions(HWND windowHandle)
{
	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	return std::make_tuple(width, height);
}

internal MSG Win32ProcessMessage(const HWND& windowHandle)
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

internal HWND Win32InitWindow(const HINSTANCE& instance, int cmdShow, ProgramState* state)
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
		state
	);

	return windowHandle;
}

internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, int width, int height)
{
	// TODO: aspect ratio correction 
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

internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int width, int height)
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

internal HRESULT Wind32InitializeXAudio(IXAudio2* xAudio, Wind32SoundBuffer* soundBuffer)
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

	WAVEFORMATEX waveFormat = { 0 };

	waveFormat.wBitsPerSample = soundBuffer->SampleBits;
	waveFormat.nSamplesPerSec = soundBuffer->SamplesPerSecond;
	waveFormat.nChannels = soundBuffer->Channels;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;

	if (FAILED(result = xAudio->CreateSourceVoice(&soundBuffer->SourceVoice, &waveFormat)))
		return result;

	return result;
}

internal void PlayGameSound(IXAudio2SourceVoice* sourceVoice)
{
	sourceVoice->Start();
}

internal HRESULT FillSoundBuffer(IXAudio2SourceVoice* sourceVoice, GameSoundBuffer* soundBuffer)
{
	HRESULT result = {};

	XAUDIO2_BUFFER buffer = { 0 };

	buffer.pAudioData = (BYTE*)soundBuffer->BufferData;
	buffer.AudioBytes = soundBuffer->SampleCount;
	buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

	if (FAILED(sourceVoice->SubmitSourceBuffer(&buffer)))
		return result;

	return result;
}

internal real32 Win32ProcessXInputStickValues(real32 value, int16 deadZoneThreshold)
{
	real32 result = 0.f;

	if (value < -deadZoneThreshold)
		result = (real32)(value + deadZoneThreshold) / (32768.f - deadZoneThreshold);
	else if (value > deadZoneThreshold)
		result = (real32)(value + deadZoneThreshold) / (32767.f - deadZoneThreshold);

	return result;
}

internal inline ProgramState* GetAppState(HWND handle)
{
	return reinterpret_cast<ProgramState*>(GetWindowLongPtr(handle, GWLP_USERDATA));
}

internal int64 GetPerformanceFrequence()
{
	LARGE_INTEGER performanceFrequenceResult;
	QueryPerformanceFrequency(&performanceFrequenceResult);
	return performanceFrequenceResult.QuadPart;
}

internal int64 QueryPerformance()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}

LRESULT CALLBACK Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	ProgramState* programState = GetAppState(windowHandle);

	switch (message)
	{
		case WM_CREATE:
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			programState = reinterpret_cast<ProgramState*>(pCreate->lpCreateParams);
			SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)programState);
		} break;

		case WM_CLOSE:
		{
			programState->IsRunning = false;
		} break;

		case WM_DESTROY:
		{
			programState->IsRunning = false;
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
						programState->IsRunning = false;

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