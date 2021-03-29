#include <filesystem>
#include "main.h"
#include "GameFunctions.h"
#include <stdio.h>

int WINAPI wWinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE prevInstance,
	_In_ LPWSTR cmdLine,
	_In_ int showCmd
)
{
	Win32ProgramState programState = { };

	HWND windowHandle = Win32InitWindow(instance, &programState);

	if (windowHandle)
	{
		Win32GetExeFileName(&programState);

		char sourceGameCodeDLLFullPath[MAX_PATH];

		Win32BuildEXEPathFileName(&programState, "Leena.dll", sizeof(sourceGameCodeDLLFullPath), sourceGameCodeDLLFullPath);

		char tempGameCodeDLLFullPath[MAX_PATH];

		Win32BuildEXEPathFileName(&programState, "LeenaTmp.dll", sizeof(tempGameCodeDLLFullPath), tempGameCodeDLLFullPath);

		ThreadContext thread;
		// Get Window just so we dont have to remove the function for the current time
		auto [width, height] = GetWindowDimensions(windowHandle);

		programState.WindowHeight = height;
		programState.WindowWidth = width;

		// Get the performance frequence
		programState.PerformanceFrequence = Win32GetPerformanceFrequence();

		UINT desiredSchedularTimeInMs = 1;

		// Set the windows schedueler granularity to 1ms
		// Which means can we sleep for exactly 1ms?
		bool32 isSleepGranular = timeBeginPeriod(desiredSchedularTimeInMs) == TIMERR_NOERROR;

		HDC dc = GetDC(windowHandle);
		uint32 monitorRefreshRate = 60; // In HZ
		uint32 win32VRefreshRate = GetDeviceCaps(dc, VREFRESH);

		if (win32VRefreshRate > 1)
			monitorRefreshRate = win32VRefreshRate;

		uint32 gameUpdateInHz = 30; // monitorRefreshRate / 2; // In HZ
		real64 targetSecondsToAdvanceBy = 1.f / gameUpdateInHz;

		programState.RecordingState.InputRecordingIndex = 0;
		programState.RecordingState.InputPlayingIndex = 0;

		GameCode game = Win32LoadGameCode(sourceGameCodeDLLFullPath, tempGameCodeDLLFullPath);
		game.LastWriteTime = GetFileLastWriteDate(sourceGameCodeDLLFullPath);

		GameMemory gameMemory = InitGameMemory();

		programState.RecordingState.TotalMemorySize = gameMemory.PermanentStorageSize + gameMemory.TransiateStorageSize;
		programState.RecordingState.GameMemory = gameMemory.PermanentStorage;

		IXAudio2* xAudio = {};
		Wind32InitializeXAudio(xAudio);

		IXAudio2MasteringVoice* masterVoice = {};
		Wind32InitializeMasterVoice(xAudio, masterVoice);

		// Init Resolution.
		Win32ResizeDIBSection(&programState.BitmapBuffer, width, height);

		// audioBuffer.SourceVoice->Stop();

		GameInput Input[2] = { };

		GameInput* previousInput = &Input[0];
		GameInput* currentInput = &Input[1];

		GameAudioBuffer gameaudioBuffer = { };

		// Get how many cycle the cpu went through
		uint64 lastCycleCount = __rdtsc();
		// Get current cpu time
		int64 lastCounter = Win32GetWallClock();

		programState.IsRunning = true;

		real64 timeTookToRenderLastFrame = 0.0f;

		while (programState.IsRunning)
		{
			currentInput->TimeToAdvance = targetSecondsToAdvanceBy;

			FILETIME newLastWriteTIme = GetFileLastWriteDate(sourceGameCodeDLLFullPath);

			if (CompareFileTime(&newLastWriteTIme, &game.LastWriteTime) != 0)
			{
				Win32UnloadGameCode(&game);
				game = Win32LoadGameCode(sourceGameCodeDLLFullPath, tempGameCodeDLLFullPath);
				game.LastWriteTime = newLastWriteTIme;
			}

			MSG message = Win32ProcessMessage();

			// Process the keyboard input.
			GameControllerInput* oldKeyboardInput = GetController(previousInput, 0);
			GameControllerInput* newKeyboardInput = GetController(currentInput, 0);

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
				ProccessKeyboardKeys(&programState, message, newKeyboardInput);
			} break;

			default:
				break;
			}

			// Process the mouse input
			Win32GetMousePosition(windowHandle, &currentInput->Mouse);
			Win32GetMouseButtonsState(&currentInput->Mouse);

			// Process the controller input
			ProccessControllerInput(currentInput, previousInput);

			GameScreenBuffer screenBuffer =
			{
				programState.BitmapBuffer.Memory,
				programState.BitmapBuffer.Width,
				programState.BitmapBuffer.Height,
				programState.BitmapBuffer.Pitch,
				programState.BitmapBuffer.BytesPerPixel,
			};

			// See if we need to record or playback recording
			if (programState.RecordingState.InputRecordingIndex)
				Win32RecordInput(&programState.RecordingState, currentInput);

			if (programState.RecordingState.InputPlayingIndex)
				Win32PlaybackInput(&programState.RecordingState, currentInput);

			// Process game update. the game returns both a sound and draw buffer so we can use.
			game.UpdateAndRender(&thread, &gameMemory, &screenBuffer, currentInput);
			game.UpdateAudio(&thread, &gameMemory, &gameaudioBuffer);

			int64 workCounter = Win32GetWallClock();
			real64 workSecondsElapsed = GetSecondsElapsed(lastCounter, workCounter, programState.PerformanceFrequence);
			real64 timeTakenOnFrame = workSecondsElapsed;

			// Make sure we stay at the target time for each frame.
			if (timeTakenOnFrame < targetSecondsToAdvanceBy)
			{
				if (isSleepGranular)
				{
					// We substract 2 ms from the sleep incase the os doesnt wake us on time
					DWORD sleepMs = (DWORD)(1000.f * (targetSecondsToAdvanceBy - timeTakenOnFrame)) - 2;

					if (sleepMs > 0 && sleepMs < 40)
						Sleep(sleepMs);
				}

				auto testTimeTakenOnFrame = GetSecondsElapsed(lastCounter, Win32GetWallClock(), programState.PerformanceFrequence);

				// Assert(testTimeTakenOnFrame < targetSecondsPerFrams);

				while (timeTakenOnFrame < targetSecondsToAdvanceBy)
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
			// TODO: Look if i dont need to create a new source voice every frame.
			IXAudio2SourceVoice* gameSourceVoice = {};

			WAVEFORMATEX wave = Wind32InitializeWaveFormat(xAudio, gameSourceVoice, &gameaudioBuffer);

			XAUDIO2_BUFFER audioBuffer2 = {};

			Win32FillaudioBuffer(gameSourceVoice, &gameaudioBuffer, audioBuffer2);

			Win32PlayAudio(gameSourceVoice);

			Win32DrawBuffer(windowHandle, &programState.BitmapBuffer, &programState);

			// Register last counter we got
			real32 FPS = (1000.0f / msPerFrame);

			char formatBuffer[256];
			sprintf_s(formatBuffer, "%.02fms/f, %.02ff/s, (%.02fws/f)\n", msPerFrame, FPS, workSecondsElapsed * 1000.0f);
			OutputDebugStringA(formatBuffer);

			// Swap the states of the input so they persist through frames
			GameInput* temp = currentInput;
			currentInput = previousInput;
			previousInput = temp;

			int64 endCycleCount = __rdtsc();
			lastCycleCount = endCycleCount;
		}
	}

	return 0;
}

// Game
internal GameCode Win32LoadGameCode(char* sourceDLLName, char* tempDLLName)
{
	CopyFileA(sourceDLLName, tempDLLName, FALSE);

	HMODULE gameCodeHandle = LoadLibraryA(tempDLLName);

	GameCode result = { gameCodeHandle, GameUpdatAndRendereStub, GameUpdateAudioStub };

	if (gameCodeHandle)
	{
		result.UpdateAndRender = (GAMEUPDATEANDRENDER*)GetProcAddress(gameCodeHandle, "GameUpdateAndRender");
		result.UpdateAudio = (GAMEUPDATEAUDIO*)GetProcAddress(gameCodeHandle, "GameUpdateAudio");

		if (result.UpdateAndRender && result.UpdateAudio)
			result.IsValid = true;
		else
			result.IsValid = false;
	}

	return result;
}
internal void Win32UnloadGameCode(GameCode* gameCode)
{
	if (gameCode->LibraryHandle)
		FreeLibrary(gameCode->LibraryHandle);

	gameCode->IsValid = false;
	gameCode->UpdateAndRender = GameUpdatAndRendereStub;
	gameCode->UpdateAudio = GameUpdateAudioStub;
}

// Windows
internal inline Win32ProgramState* GetAppState(HWND handle)
{
	return (Win32ProgramState*)GetWindowLongPtr(handle, GWLP_USERDATA);
}
internal HWND Win32InitWindow(const HINSTANCE& instance, Win32ProgramState* state)
{
	WNDCLASS window = {};

	const wchar_t className[] = L"Leena Game Engine";

	window.lpfnWndProc = Win32WindowCallback;
	window.hInstance = instance;
	window.lpszClassName = className;
	window.hCursor = LoadCursor(0, IDC_ARROW);
	window.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&window);

	HWND windowHandle = CreateWindowEx(
		0,
		window.lpszClassName,
		L"Leena Game Engine",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,//DWORD dwStyle,
		CW_USEDEFAULT, //int X,
		CW_USEDEFAULT, //int Y,
		960, //int nWidth,
		540, //int nHeight,
		NULL,
		NULL,
		instance,
		state
	);

#if Leena_Internal
	state->ShowCursor = true;
#endif

	state->PrevWP = { sizeof(state->PrevWP) };

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
	GameMemory gameMemory = {};

#if Leena_Internal
	LPVOID baseAddress = (LPVOID)Terabytes(2);
#else
	LPVOID baseAddress = 0;
#endif

	gameMemory.PermanentStorageSize = Megabytes(64);
	gameMemory.TransiateStorageSize = Gigabytes(1);

	size_t totalSize = gameMemory.PermanentStorageSize + gameMemory.TransiateStorageSize;

	gameMemory.PermanentStorage = VirtualAlloc(baseAddress, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	gameMemory.TransiateStorage = (uint8*)gameMemory.PermanentStorage + gameMemory.PermanentStorageSize;

	gameMemory.FreeFile = DebugPlatformFreeFileMemory;
	gameMemory.ReadFile = DebugPlatformReadEntireFile;
	gameMemory.WriteFile = DebugPlatformWriteEntireFile;

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
internal FILETIME GetFileLastWriteDate(const char* fileName)
{
	WIN32_FILE_ATTRIBUTE_DATA result = {};
	FILETIME lastWriteTime = {};
	if (GetFileAttributesExA(fileName, GetFileExInfoStandard, &result))
		lastWriteTime = result.ftLastWriteTime;
	return lastWriteTime;
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
	// TODO: See what we can do to play silence if needed.
	if (sourceVoice)
		sourceVoice->Start(0);
}
internal HRESULT Win32FillaudioBuffer(IXAudio2SourceVoice* sourceVoice, GameAudioBuffer* gameAudioBuffer, XAUDIO2_BUFFER& audioBuffer)
{
	HRESULT result = {};

	if (gameAudioBuffer->BufferSize > 0)
	{
		audioBuffer.AudioBytes = gameAudioBuffer->BufferSize;  //buffer containing audio data
		audioBuffer.pAudioData = (BYTE*)gameAudioBuffer->BufferData;  //size of the audio buffer in bytes
		audioBuffer.Flags = XAUDIO2_END_OF_STREAM;

		if (FAILED(sourceVoice->SubmitSourceBuffer(&audioBuffer)))
			return result;
	}

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
		result = (real32)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	else if (value > deadZoneThreshold)
		result = (real32)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));

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
internal void ProccessKeyboardKeys(Win32ProgramState* state, MSG& message, GameControllerInput* controller)
{
	uint32 vkCode = (uint32)message.wParam;

	bool32 wasDown = ((message.lParam & (1 << 30)) != 0);
	bool32 isDown = ((message.lParam & ((uint32)1 << 31)) == 0);
	bool32 altDown = message.lParam & (1 << 29);

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

		case VK_SPACE:
		{
			Win32ProccessKeyboardMessage(controller->A, isDown);
		} break;

		case VK_SHIFT:
		{
			Win32ProccessKeyboardMessage(controller->X, isDown);
		} break;

		case 'L':
		{
			if (isDown)
			{
				if (state->RecordingState.InputRecordingIndex)
				{
					Win32EndRecordingInput(&state->RecordingState);
					Win32BeginPlaybackInput(&state->RecordingState);
				}
				else
				{
					Win32BeginRecordingInput(&state->RecordingState);
				}
			}
		} break;

		case 'K':
		{
			if (isDown)
			{
				Win32EndPlaybackInput(&state->RecordingState);
			}
		} break;

		case VK_RETURN:
		{
			if (altDown && isDown)
			{
				ToggleFullScreen(state, message.hwnd, &state->PrevWP);
			}
		} break;

		case 'P':
		{
			if (isDown && !state->RecordingState.InputRecordingIndex)
			{
				Win32BeginPlaybackInput(&state->RecordingState);
			}
		} break;

		default:
		{

		} break;
		}
}
internal void Win32ProccessKeyboardMessage(GameButtonState& state, bool32 isPressed)
{
	if (state.EndedDown != isPressed)
	{
		state.EndedDown = isPressed;
		++state.HalfTransitionCount;
	}
}
internal void Win32GetMousePosition(HWND windowHandle, MouseInput* mouse)
{
	POINT mousePos = {};

	if (GetCursorPos(&mousePos))
	{
		if (ScreenToClient(windowHandle, &mousePos))
		{
			if (mousePos.x > 0)
				mouse->X = mousePos.x;
			if (mousePos.y > 0)
				mouse->Y = mousePos.y;
		}
	}
}
internal void Win32GetMouseButtonsState(MouseInput* mouse)
{
	Win32ProccessKeyboardMessage(mouse->LeftButton, GetKeyState(VK_LBUTTON) & (1 << 15));
	Win32ProccessKeyboardMessage(mouse->RightButton, GetKeyState(VK_RBUTTON) & (1 << 15));
}

// Graphics
internal WindowDimensions GetWindowDimensions(HWND windowHandle)
{
	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	return { width, height };
}
internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int32 width, int32 height)
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
internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, Win32ProgramState* state)
{
	if (state->IsFullScreen)
	{
		StretchDIBits(
			deviceContext,
			// Window Size - Destination
			0, 0, state->WindowWidth, state->WindowHeight,
			// Buffer Size - Source 
			0, 0, bitmapBuffer->Width, bitmapBuffer->Height,
			bitmapBuffer->Memory,
			&bitmapBuffer->Info,
			DIB_RGB_COLORS,
			SRCCOPY
		);
	}
	else
	{
		int offsetX = 10;
		int offsetY = 10;

		PatBlt(deviceContext, 0, 0, state->WindowWidth, offsetY, BLACKNESS);
		PatBlt(deviceContext, 0, offsetY + bitmapBuffer->Height, state->WindowWidth, state->WindowHeight, BLACKNESS);
		PatBlt(deviceContext, 0, 0, offsetX, state->WindowHeight, BLACKNESS);
		PatBlt(deviceContext, offsetX + bitmapBuffer->Width, 0, state->WindowWidth, state->WindowHeight, BLACKNESS);

		StretchDIBits(
			deviceContext,
			// Window Size - Destination
			offsetX, offsetY, bitmapBuffer->Width, bitmapBuffer->Height,
			// Buffer Size - Source 
			0, 0, bitmapBuffer->Width, bitmapBuffer->Height,
			bitmapBuffer->Memory,
			&bitmapBuffer->Info,
			DIB_RGB_COLORS,
			SRCCOPY
		);
	}
}
internal void Win32DrawBuffer(const HWND& windowHandle, Win32BitmapBuffer* buffer, Win32ProgramState* state)
{
	HDC deviceContext = GetDC(windowHandle);
	Win32DisplayBufferInWindow(buffer, deviceContext, state);
	ReleaseDC(windowHandle, deviceContext);
}

// Recording 
internal void Win32BeginRecordingInput(Win32RecordState* state)
{
	state->InputRecordingIndex = 1;
	const char* fileName = "recordingState.lrs";
	state->RecordingFileHandle = CreateFileA(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	DWORD bytesToWrite = (DWORD)state->TotalMemorySize;
	Assert(bytesToWrite == state->TotalMemorySize);
	DWORD bytesWritten;
	WriteFile(state->RecordingFileHandle, state->GameMemory, bytesToWrite, &bytesWritten, 0);
}
internal void Win32EndRecordingInput(Win32RecordState* state)
{
	state->InputRecordingIndex = 0;
	CloseHandle(state->RecordingFileHandle);
}
internal BOOL Win32BeginPlaybackInput(Win32RecordState* state)
{
	state->InputPlayingIndex = 1;
	const char* fileName = "recordingState.lrs";
	state->PlaybackFileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	DWORD bytesToRead = (DWORD)state->TotalMemorySize;
	Assert(bytesToRead == state->TotalMemorySize);
	DWORD bytesRead;
	return ReadFile(state->PlaybackFileHandle, state->GameMemory, bytesToRead, &bytesRead, 0);
}
internal void Win32EndPlaybackInput(Win32RecordState* state)
{
	state->InputPlayingIndex = 0;
	CloseHandle(state->PlaybackFileHandle);
}
internal void Win32RecordInput(Win32RecordState* state, GameInput* input)
{
	DWORD bytesWritten;
	WriteFile(state->RecordingFileHandle, input, sizeof(*input), &bytesWritten, 0);
}
internal BOOL Win32PlaybackInput(Win32RecordState* state, GameInput* input)
{
	DWORD bytesRead = 0;

	if (ReadFile(state->PlaybackFileHandle, input, sizeof(*input), &bytesRead, 0))
	{
		if (bytesRead == 0)
		{
			Win32EndPlaybackInput(state);
			Win32BeginPlaybackInput(state);
			return ReadFile(state->PlaybackFileHandle, input, sizeof(*input), &bytesRead, 0);
		}
	}

	return FALSE;
}


LRESULT CALLBACK Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	Win32ProgramState* programState = GetAppState(windowHandle);

	switch (message)
	{
	case WM_CREATE:
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		programState = (Win32ProgramState*)pCreate->lpCreateParams;
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

	case WM_SETCURSOR:
	{
		if (programState->ShowCursor)
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}
		else
		{
			SetCursor(NULL);
		}
	} break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 vkCode = (uint32)wParam;

		bool32 wasDown = ((lParam & (1 << 30)) != 0);
		bool32 isDown = ((lParam & ((uint32)1 << 31)) == 0);
		bool32 altDown = ((lParam & (1 << 29)) != 0);

		switch (vkCode)
		{
		case VK_F4:
		{
			// Is alt button held down
			if (altDown)
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
		Win32DisplayBufferInWindow(&programState->BitmapBuffer, deviceContext, programState);
		EndPaint(windowHandle, &paint);
	} break;

	default:
	{
		result = DefWindowProc(windowHandle, message, wParam, lParam);
	} break;
	}

	return result;
}

internal void Win32GetExeFileName(Win32ProgramState* State)
{
	DWORD SizeOfFilename = GetModuleFileNameA(0, State->ExeFileName, sizeof(State->ExeFileName));

	State->OnePastLastEXEFileNameSlash = State->ExeFileName;

	for (char* Scan = State->ExeFileName; *Scan; ++Scan)
	{
		if (*Scan == '\\')
		{
			State->OnePastLastEXEFileNameSlash = Scan + 1;
		}
	}
}

internal void Win32BuildEXEPathFileName(Win32ProgramState* State, const char* FileName, int DestCount, char* Dest)
{
	ConCatStrings(State->OnePastLastEXEFileNameSlash - State->ExeFileName, State->ExeFileName,
		StringLength(FileName), FileName,
		DestCount, Dest);
}


internal void ConCatStrings(
	size_t sourceACount, const char* sourceA,
	size_t sourceBCount, const char* sourceB,
	size_t destCount, char* dest)
{
	Assert(sourceACount + sourceBCount < destCount);

	for (int index = 0; index < sourceACount; ++index)
	{
		*dest++ = *sourceA++;
	}

	for (int index = 0; index < sourceBCount; ++index)
	{
		*dest++ = *sourceB++;
	}

	*dest++ = 0;
}

internal int StringLength(const char* string)
{
	int count = 0;

	while (*string++)
	{
		++count;
	}

	return count;
}

internal void ToggleFullScreen(Win32ProgramState* state, HWND handle, WINDOWPLACEMENT* prevWP)
{
	DWORD style = GetWindowLong(handle, GWL_STYLE);

	state->IsFullScreen = style & WS_OVERLAPPEDWINDOW;

	if (state->IsFullScreen)
	{

		MONITORINFO monitorInfo = { sizeof(monitorInfo) };

		if (GetWindowPlacement(handle, prevWP) && GetMonitorInfo(MonitorFromWindow(handle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
		{
			SetWindowLong(handle, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

			SetWindowPlacement(handle, prevWP);

			state->WindowHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
			state->WindowWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;

			SetWindowPos(handle, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, state->WindowWidth, state->WindowHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(handle, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(handle, prevWP);

		state->WindowHeight = prevWP->rcNormalPosition.bottom - prevWP->rcNormalPosition.top;
		state->WindowWidth = prevWP->rcNormalPosition.right - prevWP->rcNormalPosition.left;

		SetWindowPos(handle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}