#include <Leena.h>

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <tuple>

struct ProgramState
{
	bool IsRunning;
	std::map<Key, bool> KeysPressed;
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
internal int64 Win32GetPerformanceFrequence();
internal int64 Win32QueryPerformance();

// Audio
internal HRESULT Wind32InitializeXAudio(IXAudio2* xAudio, Wind32SoundBuffer* soundBuffer);
internal HRESULT Win32FillSoundBuffer(IXAudio2SourceVoice* sourceVoice, GameSoundBuffer* soundBuffer);
internal void Win32PlaySound(IXAudio2SourceVoice* sourceVoice);

// Input
internal void Win32ProcessDigitalButton(DWORD button, DWORD buttonBit, GameButtonState* oldState, GameButtonState* newState);

// Graphics
internal std::tuple<int, int> GetWindowDimensions(HWND windowHandle);
internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int width, int height);
internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, int width, int height);
internal void Win32DrawBuffer(const HWND& windowHandle);

GlobalVariable Win32BitmapBuffer GlobalBitmapBuffer;

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance, PWSTR cmdLine, int showCommand)
{
	ProgramState programState = { };

	HWND windowHandle = Win32InitWindow(instance, showCommand, &programState);

	if (windowHandle)
	{
		// Get the performance frequence
		int64 performanceFrequence = Win32GetPerformanceFrequence();

		// Get how many cycle the cpu went through
		uint64 lastCycleCount = __rdtsc();

		int64 lastCounter = Win32QueryPerformance();

		programState.IsRunning = true;

		GameMemory gameMemory = {};

		gameMemory.PermenantStorageSize = Megabytes(64);
		gameMemory.PermenantStorage = VirtualAlloc(0, gameMemory.PermenantStorageSize, MEM_COMMIT, PAGE_READWRITE);

		gameMemory.TransiateStorageSize = Gigabytes(4);
		gameMemory.TransiateStorage = VirtualAlloc(0, gameMemory.TransiateStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		IXAudio2* xAudio = {};

		Wind32SoundBuffer soundBuffer = {};

		soundBuffer.SamplesPerSecond = 44100; // Equals 44.1 kHz for pcm 
		soundBuffer.Channels = 2;
		soundBuffer.SampleBits = 16;

		Wind32InitializeXAudio(xAudio, &soundBuffer);

		Win32ResizeDIBSection(&GlobalBitmapBuffer, 1280, 720);

		// Uncomment when we have proper wave to play ...
		// PlayGameSound(soundBuffer.SourceVoice);

		GameInput Input[2] = {};

		GameInput* oldInput = &Input[0];
		GameInput* newInput = &Input[0];

		while (programState.IsRunning)
		{
			Win32ProcessMessage(windowHandle);

			newInput->KeysPressed = programState.KeysPressed;

			uint8 maxCount = XUSER_MAX_COUNT;

			if (maxCount > ArrayCount(newInput->Controllers))
				maxCount = ArrayCount(newInput->Controllers);

			for (uint8 controllerIndex = 0; controllerIndex < maxCount; controllerIndex++)
			{
				XINPUT_STATE controllerState;

				// If the controller is connected
				if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
				{
					GameControllerInput* oldController = &oldInput->Controllers[controllerIndex];
					GameControllerInput* newController = &newInput->Controllers[controllerIndex];

					// Controller is connected
					XINPUT_GAMEPAD* pad = &controllerState.Gamepad;

					newController->IsConnected = true;
					newController->IsAnalog = true;

					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_UP, &oldController->PadUp, &newController->PadUp);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_DOWN, &oldController->PadDown, &newController->PadDown);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_RIGHT, &oldController->PadRight, &newController->PadRight);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_LEFT, &oldController->PadLeft, &newController->PadLeft);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_START, &oldController->Start, &newController->Start);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_BACK, &oldController->Back, &newController->Back);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &oldController->RightShoulder, &newController->RightShoulder);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &oldController->LeftShoulder, &newController->LeftShoulder);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_A, &oldController->A, &newController->A);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_B, &oldController->B, &newController->B);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_X, &oldController->X, &newController->X);
					Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_Y, &oldController->Y, &newController->Y);

					// Normalize the number
					newController->StickAverageX = (real32)pad->sThumbLX / ((pad->sThumbLX < 0) ? 32768.0f : 32767.0f);
					newController->StickAverageY = (real32)pad->sThumbLY / ((pad->sThumbLX < 0) ? 32768.0f : 32767.0f);

					XINPUT_VIBRATION vibrations = {};

					if (newController->A.EndedDown)
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

			GameUpdate(&gameMemory, &screenBuffer, &gameSoundBuffer, newInput);

			Win32FillSoundBuffer(soundBuffer.SourceVoice, &gameSoundBuffer);
			Win32DrawBuffer(windowHandle);

			// Display performance counter
			uint64 endCycleCount = __rdtsc();

			int64 endCounter = Win32QueryPerformance();

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

			GameInput* temp = newInput;
			newInput = oldInput;
			oldInput = temp;
		}
	}

	return 0;
}

internal void Win32DrawBuffer(const HWND& windowHandle)
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

internal void Win32PlaySound(IXAudio2SourceVoice* sourceVoice)
{
	sourceVoice->Start();
}

internal HRESULT Win32FillSoundBuffer(IXAudio2SourceVoice* sourceVoice, GameSoundBuffer* soundBuffer)
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

internal int64 Win32GetPerformanceFrequence()
{
	LARGE_INTEGER performanceFrequenceResult;
	QueryPerformanceFrequency(&performanceFrequenceResult);
	return performanceFrequenceResult.QuadPart;
}

internal int64 Win32QueryPerformance()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}

internal void Win32ProcessDigitalButton(DWORD button, DWORD buttonBit, GameButtonState* oldState, GameButtonState* newState)
{
	newState->HalfTransitionCount = newState->HalfTransitionCount != oldState->HalfTransitionCount ? 1 : 0;
	newState->EndedDown = (button & buttonBit) == buttonBit;
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

			programState->KeysPressed[Key::Alt] = (lParam & (1 << 29)) != 0;
			programState->KeysPressed[Key::W] = vkCode == 'W';
			programState->KeysPressed[Key::A] = vkCode == 'A';
			programState->KeysPressed[Key::D] = vkCode == 'D';
			programState->KeysPressed[Key::E] = vkCode == 'E';
			programState->KeysPressed[Key::PadUp] = vkCode == VK_UP;
			programState->KeysPressed[Key::PadDown] = vkCode == VK_DOWN;
			programState->KeysPressed[Key::PadRight] = vkCode == VK_RIGHT;
			programState->KeysPressed[Key::PadLeft] = vkCode == VK_LEFT;
			programState->KeysPressed[Key::Space] = vkCode == VK_SPACE;
			programState->KeysPressed[Key::Esc] = vkCode == VK_ESCAPE;
			programState->KeysPressed[Key::Ctrl] = vkCode == VK_CONTROL;

			switch (vkCode)
			{
				case VK_F4:
				{
					// Is alt button held down
					if (programState->KeysPressed[Key::Alt])
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