#include "main.h"
#include <stdio.h>

GlobalVariable Win32BitmapBuffer GlobalBitmapBuffer;

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance, PWSTR cmdLine, int showCommand)
{
	ProgramState programState = { };

	HWND windowHandle = Win32InitWindow(instance, &programState);

	if (windowHandle)
	{
		// Get the performance frequence
		programState.PerformanceFrequence = Win32GetPerformanceFrequence();
		// Get how many cycle the cpu went through
		uint64 lastCycleCount = __rdtsc();
		// Get current cpu time
		int64 lastCounter = Win32GetWallClock();

		UINT desiredSchedularTimeInMs = 1;

		// Set the windows schedueler granularity to 1ms
		bool32 isSleepGranular = timeBeginPeriod(desiredSchedularTimeInMs) == TIMERR_NOERROR;

		uint8 monitorRefreshRate = 60; // In HZ
		uint8 gameUpdateInHz = monitorRefreshRate / 2; // In HZ
		real32 targetSecondsPerFrams = 1.f / gameUpdateInHz;

		programState.IsRunning = true;

		GameMemory gameMemory = InitGameMemory();

		IXAudio2* xAudio = {};
		Wind32InitializeXAudio(xAudio);

		IXAudio2MasteringVoice* masterVoice = {};
		Wind32InitializeMasterVoice(xAudio, masterVoice);

		// Init Resolution.
		Win32ResizeDIBSection(&GlobalBitmapBuffer, 1280, 720);

		// audioBuffer.SourceVoice->Stop();

		GameInput Input[2] = { };

		GameInput* oldInput = &Input[0];
		GameInput* newInput = &Input[1];

		GameAudioBuffer gameaudioBuffer = { };

		bool playingAudio = false;

		while (programState.IsRunning)
		{
			MSG message = Win32ProcessMessage();

			// Process the keyboard input.
			GameControllerInput* oldKeyboardInput = GetController(oldInput, 0);
			GameControllerInput* newKeyboardInput = GetController(newInput, 0);

			*newKeyboardInput = {};

			newKeyboardInput->IsConnected = true;

			// Keep the state of the down button from the past frame.
			for (int buttonIndex = 0; buttonIndex < ArrayCount(newKeyboardInput->Buttons); buttonIndex++)
				newKeyboardInput->Buttons[buttonIndex].EndedDown = oldKeyboardInput->Buttons[buttonIndex].EndedDown;

			switch (message.message)
			{
				case WM_QUIT:
				{
					programState.IsRunning = false;
				} break;

				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				case WM_KEYDOWN:
				case WM_KEYUP:
				{
					ProccessKeyboardKeys(message, newKeyboardInput);
				} break;

				default:
					break;
			}

			// Process the controller input
			ProccessControllerInput(newInput, oldInput);

			GameScreenBuffer screenBuffer =
			{
				GlobalBitmapBuffer.Memory,
				GlobalBitmapBuffer.Width,
				GlobalBitmapBuffer.Height,
				GlobalBitmapBuffer.Pitch,
			};

			// Process game update. the game returns both a sound and draw buffer so we can use.
			GameUpdate(&gameMemory, &screenBuffer, &gameaudioBuffer, newInput);

			int64 workCounter = Win32GetWallClock();
			real64 workSecondsElapsed = GetSecondsElapsed(lastCounter, workCounter, programState.PerformanceFrequence);
			real64 timeTakenOnFrame = workSecondsElapsed;

			// Make sure we stay at the target time for each frame.
			if (timeTakenOnFrame < targetSecondsPerFrams)
			{
				if (isSleepGranular)
				{
					// We substract 2 ms from the sleep incase the os doesnt wake us on time
					DWORD sleepMs = (DWORD)(1000.f * (targetSecondsPerFrams - timeTakenOnFrame)) - 2;

					if (sleepMs > 0)
						Sleep(sleepMs);
				}

				auto testTimeTakenOnFrame = GetSecondsElapsed(lastCounter, Win32GetWallClock(), programState.PerformanceFrequence);

				Assert(testTimeTakenOnFrame < targetSecondsPerFrams);

				while (timeTakenOnFrame < targetSecondsPerFrams)
				{
					timeTakenOnFrame = GetSecondsElapsed(lastCounter, Win32GetWallClock(), programState.PerformanceFrequence);
				}
			}
			else
			{
				// missed a frame
				OutputDebugStringA("Missed a frame");
			}

			// Display performance counter
			int64 endCounter = Win32GetWallClock();
			real32 msPerFrame = 1000.0f * GetSecondsElapsed(lastCounter, endCounter, programState.PerformanceFrequence);
			lastCounter = endCounter;

			// We fille the sound and draw buffers we got from the game.
			Win32DrawBuffer(windowHandle);

			if (!playingAudio || newInput->Controllers[0].MoveUp.EndedDown)
			{
				IXAudio2SourceVoice* gameSourceVoice = {};

				auto wave = Wind32InitializeWaveFormat(xAudio, gameSourceVoice, &gameaudioBuffer);

				XAUDIO2_BUFFER audioBuffer2 = {};

				Win32FillaudioBuffer(gameSourceVoice, &gameaudioBuffer, audioBuffer2);

				Win32PlayAudio(gameSourceVoice);

				playingAudio = true;
			}

			// Register last counter we got
			real32 FPS = (1000.0f / msPerFrame);

			char formatBuffer[256];
			sprintf_s(formatBuffer, "%.02fms/f, %.02ff/s, (%.02fws/f)\n", msPerFrame, FPS, workSecondsElapsed * 1000.0f);
			OutputDebugStringA(formatBuffer);

			// Swap the states of the input so they persist through frames
			GameInput* temp = newInput;
			newInput = oldInput;
			oldInput = temp;

			int64 endCycleCount = __rdtsc();
			int64 cyclesElapsed = endCycleCount - lastCycleCount;
			lastCycleCount = endCycleCount;
		}
	}

	return 0;
}

// Windows
internal inline ProgramState* GetAppState(HWND handle)
{
	return reinterpret_cast<ProgramState*>(GetWindowLongPtr(handle, GWLP_USERDATA));
}
internal HWND Win32InitWindow(const HINSTANCE& instance, ProgramState* state)
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
internal MSG Win32ProcessMessage()
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
internal GameMemory InitGameMemory()
{
	GameMemory gameMemory;

	#if Leena_Internal
	LPVOID baseAddress = (LPVOID)Terabytes(2);
	#else
	LPVOID baseAddress = 0;
	#endif

	gameMemory.PermenantStorageSize = Megabytes(64);
	gameMemory.TransiateStorageSize = Gigabytes(4);

	uint64 totalSize = gameMemory.PermenantStorageSize + gameMemory.TransiateStorageSize;

	gameMemory.PermenantStorage = VirtualAlloc(baseAddress, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	gameMemory.TransiateStorage = (uint8*)gameMemory.PermenantStorage + gameMemory.PermenantStorageSize;

	return gameMemory;
}
internal inline int64 Win32GetPerformanceFrequence()
{
	LARGE_INTEGER performanceFrequenceResult;
	QueryPerformanceFrequency(&performanceFrequenceResult);
	return performanceFrequenceResult.QuadPart;
}
internal inline int64 Win32GetWallClock()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}
internal real32 GetSecondsElapsed(uint64 start, uint64 end, uint64 performanceFrequence)
{
	return (real32)(end - start) / (real32)performanceFrequence;
}

// Audio
internal HRESULT Wind32InitializeXAudio(IXAudio2*& xAudio)
{
	// TODO: UncoInitialize on error.
	HRESULT result;

	if (FAILED(result = CoInitialize(NULL)))
		return result;

	if (FAILED(result = XAudio2Create(&xAudio)))
		return result;

	return result;
}
internal HRESULT Wind32InitializeMasterVoice(IXAudio2* xAudio, IXAudio2MasteringVoice*& masteringVoice)
{
	HRESULT result;

	if (FAILED(result = xAudio->CreateMasteringVoice(&masteringVoice)))
		return result;

	return result;
}
internal WAVEFORMATEX Wind32InitializeWaveFormat(IXAudio2* xAudio, IXAudio2SourceVoice*& sourceVoice, GameAudioBuffer* audioBuffer)
{
	HRESULT result;

	WAVEFORMATEX waveFormat;

	waveFormat.wBitsPerSample = audioBuffer->BitsPerSample;
	waveFormat.nSamplesPerSec = audioBuffer->SamplesPerSec;
	waveFormat.nChannels = audioBuffer->Channels;
	waveFormat.nAvgBytesPerSec = audioBuffer->AvgBytesPerSec;
	waveFormat.wFormatTag = audioBuffer->FormatTag;
	waveFormat.nBlockAlign = audioBuffer->BlockAlign;

	// What to do if fails?
	// if (FAILED());
	result = xAudio->CreateSourceVoice(&sourceVoice, &waveFormat);

	return waveFormat;
}
internal void Win32PlayAudio(IXAudio2SourceVoice* sourceVoice)
{
	sourceVoice->Start(0);
}
internal HRESULT Win32FillaudioBuffer(IXAudio2SourceVoice* sourceVoice, GameAudioBuffer* gameAudioBuffer, XAUDIO2_BUFFER& audioBuffer)
{
	HRESULT result = {};

	audioBuffer.AudioBytes = gameAudioBuffer->BufferSize;  //buffer containing audio data
	audioBuffer.pAudioData = (BYTE*)gameAudioBuffer->BufferData;  //size of the audio buffer in bytes
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;

	if (FAILED(sourceVoice->SubmitSourceBuffer(&audioBuffer)))
		return result;

	return result;
}

// Input
internal void Win32ProcessDigitalButton(DWORD button, DWORD buttonBit, GameButtonState* oldState, GameButtonState* newState)
{
	newState->HalfTransitionCount = newState->HalfTransitionCount != oldState->HalfTransitionCount ? 1 : 0;
	newState->EndedDown = (button & buttonBit) == buttonBit;
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
internal void ProccessControllerInput(GameInput* newInput, GameInput* oldInput)
{
	uint8 maxCount = XUSER_MAX_COUNT;

	if (maxCount > ArrayCount(newInput->Controllers) - 1)
		maxCount = (ArrayCount(newInput->Controllers) - 1);

	for (uint8 controllerIndex = 0; controllerIndex < maxCount; controllerIndex++)
	{
		XINPUT_STATE controllerState;

		GameControllerInput* oldController = GetController(oldInput, controllerIndex + 1);
		GameControllerInput* newController = GetController(newInput, controllerIndex + 1);

		// If the controller is connected
		if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
		{
			newController->IsConnected = true;

			// Controller is connected
			XINPUT_GAMEPAD* pad = &controllerState.Gamepad;

			newController->IsConnected = true;
			newController->IsAnalog = true;

			newController->LeftTrigger = Win32CalculateTriggerValue(pad->bLeftTrigger);
			newController->RightTrigger = Win32CalculateTriggerValue(pad->bRightTrigger);

			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_UP, &oldController->DpadUp, &newController->DpadUp);
			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_DOWN, &oldController->DpadDown, &newController->DpadDown);
			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_RIGHT, &oldController->DpadRight, &newController->DpadRight);
			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_LEFT, &oldController->DpadLeft, &newController->DpadLeft);

			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_START, &oldController->Start, &newController->Start);
			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_BACK, &oldController->Back, &newController->Back);

			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &oldController->RightShoulder, &newController->RightShoulder);
			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &oldController->LeftShoulder, &newController->LeftShoulder);

			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_A, &oldController->A, &newController->A);
			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_B, &oldController->B, &newController->B);
			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_X, &oldController->X, &newController->X);
			Win32ProcessDigitalButton(pad->wButtons, XINPUT_GAMEPAD_Y, &oldController->Y, &newController->Y);

			// Normalize the numbers for sticks
			real32 threshHold = 0.5f;

			newController->LeftStickAverageX = Win32ProcessXInputStickValues(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			newController->LeftStickAverageY = Win32ProcessXInputStickValues(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

			newController->RightStickAverageX = Win32ProcessXInputStickValues(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			newController->RightStickAverageY = Win32ProcessXInputStickValues(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

			Win32ProcessDigitalButton(newController->LeftStickAverageX < -threshHold ? 1 : 0, 1, & oldController->MoveLeft, & newController->MoveLeft);
			Win32ProcessDigitalButton(newController->LeftStickAverageX > threshHold ? 1 : 0, 1, &oldController->MoveRight, &newController->MoveRight);

			Win32ProcessDigitalButton(newController->LeftStickAverageY < -threshHold ? 1 : 0, 1, & oldController->MoveDown, & newController->MoveDown);
			Win32ProcessDigitalButton(newController->LeftStickAverageY > threshHold ? 1 : 0, 1, &oldController->MoveUp, &newController->MoveUp);

			XINPUT_VIBRATION vibrations = {};

			if (newController->A.EndedDown)
			{
				vibrations.wLeftMotorSpeed = 60000;
				vibrations.wRightMotorSpeed = 60000;
			}

			XInputSetState(controllerIndex, &vibrations);
		}
		else
		{
			newController->IsConnected = false;
		}
	}
}
internal real32 Win32CalculateTriggerValue(real32 triggerValue)
{
	return triggerValue > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? triggerValue / 255 : 0;
}
internal void ProccessKeyboardKeys(MSG& message, GameControllerInput* controller)
{
	uint32 vkCode = static_cast<uint32>(message.wParam);

	bool wasDown = ((message.lParam & (1 << 30)) != 0);
	bool isDown = ((message.lParam & (static_cast<uint32>(1) << 31)) == 0);

	if (isDown != wasDown)
		switch (vkCode)
		{
			case 'W':
			{
				Win32ProccessKeyboardMessage(controller->MoveUp, isDown);
			} break;

			case 'A':
			{
				Win32ProccessKeyboardMessage(controller->MoveLeft, isDown);
			} break;

			case 'D':
			{
				Win32ProccessKeyboardMessage(controller->MoveRight, isDown);
			} break;

			case 'S':
			{
				Win32ProccessKeyboardMessage(controller->MoveDown, isDown);
			} break;

			case 'Q':
			{
				Win32ProccessKeyboardMessage(controller->LeftShoulder, isDown);
			} break;

			case 'E':
			{
				Win32ProccessKeyboardMessage(controller->RightShoulder, isDown);
			} break;

			default:
			{

			} break;
		}
}
internal void Win32ProccessKeyboardMessage(GameButtonState& state, bool isPressed)
{
	state.EndedDown = isPressed;
	++state.HalfTransitionCount;
}

// Graphics
internal std::tuple<int, int> GetWindowDimensions(HWND windowHandle)
{
	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	return std::make_tuple(width, height);
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
internal void Win32DrawBuffer(const HWND& windowHandle)
{
	HDC deviceContext = GetDC(windowHandle);
	auto [width, height] = GetWindowDimensions(windowHandle);
	Win32DisplayBufferInWindow(&GlobalBitmapBuffer, deviceContext, width, height);
	ReleaseDC(windowHandle, deviceContext);
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
			uint32 vkCode = static_cast<uint32>(wParam);

			bool wasDown = ((lParam & (1 << 30)) != 0);
			bool isDown = ((lParam & (static_cast<uint32>(1) << 31)) == 0);

			switch (vkCode)
			{
				case VK_F4:
				{
					// Is alt button held down
					//if (programState->KeysPressed[Key::Alt])
					//	programState->IsRunning = false;
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