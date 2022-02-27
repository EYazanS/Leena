#include "main.h"

GlobalVariable Win32ProgramState programState = {};

#include "Win32Services.cpp"
#include "GameFunctions.h"

int WINAPI wWinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE prevInstance,
	_In_ LPWSTR cmdLine,
	_In_ int showCmd)
{
	HWND windowHandle = Win32InitWindow(instance, &programState);

	if (windowHandle)
	{
		Win32GetExeFileName(&programState);

		char sourceGameCodeDLLFullPath[MAX_PATH];

		Win32BuildEXEPathFileName(&programState, "Leena.dll", sizeof(sourceGameCodeDLLFullPath), sourceGameCodeDLLFullPath);

		char tempGameCodeDLLFullPath[MAX_PATH];

		Win32BuildEXEPathFileName(&programState, "LeenaTmp.dll", sizeof(tempGameCodeDLLFullPath), tempGameCodeDLLFullPath);

		char gameCodeLockFullPath[MAX_PATH];
		Win32BuildEXEPathFileName(&programState, "lock.tmp", sizeof(gameCodeLockFullPath), gameCodeLockFullPath);

		ThreadContext thread;
		// Get Window just so we dont have to remove the function for the current time
		auto [width, height] = GetWindowDimensions(windowHandle);

		programState.WindowHeight = height;
		programState.WindowWidth = width;

		// Get the performance frequence
		programState.PerformanceFrequence = Win32GetPerformanceFrequence();

		UINT desiredSchedularTimeInMs = 1;

		// Set the windows schedueler granularity to 1ms
		// Which means can we sleep for exactly 1ms
		b32 isSleepGranular = timeBeginPeriod(desiredSchedularTimeInMs) == TIMERR_NOERROR;

		HDC dc = GetDC(windowHandle);
		u32 monitorRefreshRate = 60; // In HZ
		u32 win32VRefreshRate = GetDeviceCaps(dc, VREFRESH);

		if (win32VRefreshRate > 1)
			monitorRefreshRate = win32VRefreshRate;

		u32 gameUpdateInHz = 30; // monitorRefreshRate / 2; // In HZ
		r64 targetSecondsToAdvanceBy = 1.f / gameUpdateInHz;

		programState.RecordingState.InputRecordingIndex = 0;
		programState.RecordingState.InputPlayingIndex = 0;

		GameCode game = Win32LoadGameCode(sourceGameCodeDLLFullPath, tempGameCodeDLLFullPath, gameCodeLockFullPath);

		GameMemory gameMemory = InitGameMemory();

		programState.RecordingState.TotalMemorySize = gameMemory.PermanentStorageSize + gameMemory.TransiateStorageSize;
		programState.RecordingState.GameMemory = gameMemory.PermanentStorage;

		IXAudio2 *xAudio = {};
		Wind32InitializeXAudio(xAudio);

		IXAudio2MasteringVoice *masterVoice = {};
		Wind32InitializeMasterVoice(xAudio, masterVoice);

		// Init Resolution.
		Win32ResizeDIBSection(&programState.BitmapBuffer, width, height);

		// audioBuffer.SourceVoice->Stop();

		GameInput Input[2] = {};

		GameInput *previousInput = &Input[0];
		GameInput *currentInput = &Input[1];

		AudioBuffer gameaudioBuffer = {};

		// Get how many cycle the cpu went through
		u64 lastCycleCount = __rdtsc();
		// Get current cpu time
		i64 lastCounter = Win32GetWallClock();

		programState.IsRunning = true;

		r64 timeTookToRenderLastFrame = 0.0f;

		while (programState.IsRunning)
		{
			currentInput->TimeToAdvance = targetSecondsToAdvanceBy;

			FILETIME newLastWriteTIme = GetFileLastWriteDate(sourceGameCodeDLLFullPath);

			if (CompareFileTime(&newLastWriteTIme, &game.LastWriteTime) != 0)
			{
				Win32UnloadGameCode(&game);
				game = Win32LoadGameCode(sourceGameCodeDLLFullPath, tempGameCodeDLLFullPath, gameCodeLockFullPath);
				game.LastWriteTime = newLastWriteTIme;
			}

			MSG message = Win32ProcessMessage();

			// Process the keyboard input.
			GameInput *oldInput = previousInput;
			GameInput *newInput = currentInput;

			*newInput = {};

			newInput->TimeToAdvance = targetSecondsToAdvanceBy;

			for (int stateIndex = 0; stateIndex < ArrayCount(newInput->States); stateIndex++)
				newInput->States[stateIndex].EndedDown = oldInput->States[stateIndex].EndedDown;

			switch (message.message)
			{
			case WM_QUIT:
			{
				programState.IsRunning = false;
			}
			break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				ProccessKeyboardKeys(&programState, message, newInput);
			}
			break;

			default:
				break;
			}

			// Process the mouse input
			Win32GetMousePosition(windowHandle, &currentInput->Mouse);
			Win32GetMouseButtonsState(currentInput);

			// Process the controller input
			ProccessControllerInput(currentInput, previousInput);

			ScreenBuffer screenBuffer =
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

			i64 workCounter = Win32GetWallClock();
			r64 workSecondsElapsed = GetSecondsElapsed(lastCounter, workCounter, programState.PerformanceFrequence);
			r64 timeTakenOnFrame = workSecondsElapsed;

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
				OutputDebugStringA("Missed a frame\n");
			}

			// Display performance counter
			i64 endCounter = Win32GetWallClock();
			r32 msPerFrame = 1000.0f * GetSecondsElapsed(lastCounter, endCounter, programState.PerformanceFrequence);
			lastCounter = endCounter;

			// We fille the sound and draw buffers we got from the game.
			// TODO: Look if i dont need to create a new source voice every frame.
			IXAudio2SourceVoice *gameSourceVoice = {};

			WAVEFORMATEX wave = Wind32InitializeWaveFormat(xAudio, gameSourceVoice, &gameaudioBuffer);

			XAUDIO2_BUFFER audioBuffer2 = {};

			Win32FillaudioBuffer(gameSourceVoice, &gameaudioBuffer, audioBuffer2);

			Win32PlayAudio(gameSourceVoice);

			Win32DrawBuffer(windowHandle, &programState.BitmapBuffer, &programState);

			// Register last counter we got
			r32 FPS = (1000.0f / msPerFrame);

			char formatBuffer[256];
			sprintf_s(formatBuffer, "%.02fms/f, %.02ff/s, (%.02fws/f)\n", msPerFrame, FPS, workSecondsElapsed * 1000.0f);
			OutputDebugStringA(formatBuffer);

			// Swap the states of the input so they persist through frames
			GameInput *temp = currentInput;
			currentInput = previousInput;
			previousInput = temp;

			i64 endCycleCount = __rdtsc();
			lastCycleCount = endCycleCount;
		}
	}

	return 0;
}

// Game
internal GameCode Win32LoadGameCode(char *sourceDLLName, char *tempDLLName, char *lockFileName)
{
	GameCode result = {};

	WIN32_FILE_ATTRIBUTE_DATA Ignored;

	if (!GetFileAttributesExA(lockFileName, GetFileExInfoStandard, &Ignored))
	{
		result.LastWriteTime = GetFileLastWriteDate(sourceDLLName);

		CopyFileA(sourceDLLName, tempDLLName, FALSE);

		result.LibraryHandle = LoadLibraryA(tempDLLName);

		if (result.LibraryHandle)
		{
			result.UpdateAndRender = (GAMEUPDATEANDRENDER *)GetProcAddress(result.LibraryHandle, "GameUpdateAndRender");
			result.UpdateAudio = (GAMEUPDATEAUDIO *)GetProcAddress(result.LibraryHandle, "GameUpdateAudio");
			result.IsValid = result.UpdateAndRender && result.UpdateAudio;
		}
	}

	if (!result.IsValid)
	{
		result.UpdateAndRender = 0;
		result.UpdateAudio = 0;
	}

	return result;
}

internal void Win32UnloadGameCode(GameCode *gameCode)
{
	if (gameCode->LibraryHandle)
		FreeLibrary(gameCode->LibraryHandle);

	gameCode->IsValid = false;
	gameCode->UpdateAndRender = GameUpdatAndRendereStub;
	gameCode->UpdateAudio = GameUpdateAudioStub;
}

// Windows
internal inline Win32ProgramState *GetAppState(HWND handle)
{
	return (Win32ProgramState *)GetWindowLongPtr(handle, GWLP_USERDATA);
}
internal HWND Win32InitWindow(const HINSTANCE &instance, Win32ProgramState *state)
{
	WNDCLASSA window = {};

	LPCSTR className = "Leena Game Engine";

	window.lpfnWndProc = Win32WindowCallback;
	window.hInstance = instance;
	window.lpszClassName = className;
	window.hCursor = LoadCursor(0, IDC_ARROW);
	window.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

	RegisterClassA(&window);

	HWND windowHandle = CreateWindowExA(
		0,
		window.lpszClassName,
		className,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, // DWORD dwStyle,
		CW_USEDEFAULT,					  // int X,
		CW_USEDEFAULT,					  // int Y,
		960,							  // int nWidth,
		540,							  // int nHeight,
		NULL,
		NULL,
		instance,
		state);

#if Leena_Internal
	state->ShowCursor = true;
#endif

	state->PrevWP = {sizeof(state->PrevWP)};

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

	gameMemory.PermanentStorageSize = Megabytes(256);
	gameMemory.TransiateStorageSize = Gigabytes(1);

	size_t totalSize = gameMemory.PermanentStorageSize + gameMemory.TransiateStorageSize;

	gameMemory.PermanentStorage = VirtualAlloc(baseAddress, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	gameMemory.TransiateStorage = (u8 *)gameMemory.PermanentStorage + gameMemory.PermanentStorageSize;

	gameMemory.FreeFile = DebugPlatformFreeFileMemory;
	gameMemory.ReadFile = DebugPlatformReadEntireFile;
	gameMemory.WriteFile = DebugPlatformWriteEntireFile;

	return gameMemory;
}
internal inline i64 Win32GetPerformanceFrequence()
{
	LARGE_INTEGER performanceFrequenceResult;
	QueryPerformanceFrequency(&performanceFrequenceResult);
	return performanceFrequenceResult.QuadPart;
}
internal inline i64 Win32GetWallClock()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}
internal r32 GetSecondsElapsed(u64 start, u64 end, u64 performanceFrequence)
{
	return (r32)(end - start) / (r32)performanceFrequence;
}
internal FILETIME GetFileLastWriteDate(const char *fileName)
{
	WIN32_FILE_ATTRIBUTE_DATA result = {};
	FILETIME lastWriteTime = {};
	if (GetFileAttributesExA(fileName, GetFileExInfoStandard, &result))
		lastWriteTime = result.ftLastWriteTime;
	return lastWriteTime;
}

// Audio
internal HRESULT Wind32InitializeXAudio(IXAudio2 *&xAudio)
{
	// TODO: UncoInitialize on error.
	HRESULT result;

	if (FAILED(result = CoInitialize(NULL)))
		return result;

	if (FAILED(result = XAudio2Create(&xAudio)))
		return result;

	return result;
}
internal HRESULT Wind32InitializeMasterVoice(IXAudio2 *xAudio, IXAudio2MasteringVoice *&masteringVoice)
{
	HRESULT result;

	if (FAILED(result = xAudio->CreateMasteringVoice(&masteringVoice)))
		return result;

	return result;
}
internal WAVEFORMATEX Wind32InitializeWaveFormat(IXAudio2 *xAudio, IXAudio2SourceVoice *&sourceVoice, AudioBuffer *audioBuffer)
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
internal void Win32PlayAudio(IXAudio2SourceVoice *sourceVoice)
{
	// TODO: See what we can do to play silence if needed.
	if (sourceVoice)
		sourceVoice->Start(0);
}
internal HRESULT Win32FillaudioBuffer(IXAudio2SourceVoice *sourceVoice, AudioBuffer *gameAudioBuffer, XAUDIO2_BUFFER &audioBuffer)
{
	HRESULT result = {};

	if (gameAudioBuffer->BufferSize > 0)
	{
		audioBuffer.AudioBytes = gameAudioBuffer->BufferSize;		  // buffer containing audio data
		audioBuffer.pAudioData = (BYTE *)gameAudioBuffer->BufferData; // size of the audio buffer in bytes
		audioBuffer.Flags = XAUDIO2_END_OF_STREAM;

		if (FAILED(sourceVoice->SubmitSourceBuffer(&audioBuffer)))
			return result;
	}

	return result;
}

// Input
internal r32 Win32ProcessXInputStickValues(r32 value, i16 deadZoneThreshold)
{
	r32 result = 0.f;

	if (value < -deadZoneThreshold)
		result = (r32)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	else if (value > deadZoneThreshold)
		result = (r32)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));

	return result;
}
internal void ProccessControllerInput(GameInput *newInput, GameInput *oldInput)
{
	XINPUT_STATE controllerState;

	ControllerInput *oldController = &oldInput->Controller;
	ControllerInput *newController = &newInput->Controller;

	// If the controller is connected
	if (XInputGetState(0, &controllerState) == ERROR_SUCCESS)
	{
		newController->IsConnected = true;

		// Controller is connected
		XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

		newController->IsConnected = true;
		newController->IsAnalog = true;

		newController->LeftTrigger = Win32CalculateTriggerValue(pad->bLeftTrigger);
		newController->RightTrigger = Win32CalculateTriggerValue(pad->bRightTrigger);

		Win32ProccessInput(newInput, KeyAction::ActionUp, (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) == XINPUT_GAMEPAD_DPAD_UP);
		Win32ProccessInput(newInput, KeyAction::ActionDown, (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) == XINPUT_GAMEPAD_DPAD_DOWN);
		Win32ProccessInput(newInput, KeyAction::ActionRight, (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) == XINPUT_GAMEPAD_DPAD_RIGHT);
		Win32ProccessInput(newInput, KeyAction::ActionLeft, (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) == XINPUT_GAMEPAD_DPAD_LEFT);

		Win32ProccessInput(newInput, KeyAction::Start, (pad->wButtons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START);
		Win32ProccessInput(newInput, KeyAction::Back, (pad->wButtons & XINPUT_GAMEPAD_BACK) == XINPUT_GAMEPAD_BACK);

		Win32ProccessInput(newInput, KeyAction::Run, (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) == XINPUT_GAMEPAD_RIGHT_SHOULDER);
		Win32ProccessInput(newInput, KeyAction::LS, (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) == XINPUT_GAMEPAD_LEFT_SHOULDER);

		Win32ProccessInput(newInput, KeyAction::Jump, (pad->wButtons & XINPUT_GAMEPAD_A) == XINPUT_GAMEPAD_A);
		Win32ProccessInput(newInput, KeyAction::B, (pad->wButtons & XINPUT_GAMEPAD_B) == XINPUT_GAMEPAD_B);
		Win32ProccessInput(newInput, KeyAction::X, (pad->wButtons & XINPUT_GAMEPAD_X) == XINPUT_GAMEPAD_X);
		Win32ProccessInput(newInput, KeyAction::Y, (pad->wButtons & XINPUT_GAMEPAD_Y) == XINPUT_GAMEPAD_Y);

		// Normalize the numbers for sticks
		r32 threshHold = 0.5f;

		newController->LeftStickAverageX = Win32ProcessXInputStickValues(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		newController->LeftStickAverageY = Win32ProcessXInputStickValues(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

		newController->RightStickAverageX = Win32ProcessXInputStickValues(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		newController->RightStickAverageY = Win32ProcessXInputStickValues(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

#if 0
		XINPUT_VIBRATION vibrations = {};

		if (newController->A.EndedDown)
		{
			vibrations.wLeftMotorSpeed = 60000;
			vibrations.wRightMotorSpeed = 60000;
		}

		XInputSetState(0, &vibrations);
#endif // 0
	}
	else
	{
		newController->IsConnected = false;
	}
}
internal r32 Win32CalculateTriggerValue(r32 triggerValue)
{
	return triggerValue > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? triggerValue / 255 : 0;
}
internal void ProccessKeyboardKeys(Win32ProgramState *state, MSG &message, GameInput *input)
{
	u32 vkCode = (u32)message.wParam;

	b32 wasDown = ((message.lParam & (1 << 30)) != 0);
	b32 isDown = ((message.lParam & ((u32)1 << 31)) == 0);
	b32 altDown = message.lParam & (1 << 29);

	if (isDown != wasDown)
		switch (vkCode)
		{
		case 'W':
		{
			Win32ProccessInput(input, KeyAction::MoveUp, isDown);
		}
		break;

		case 'A':
		{
			Win32ProccessInput(input, KeyAction::MoveLeft, isDown);
		}
		break;

		case 'D':
		{
			Win32ProccessInput(input, KeyAction::MoveRight, isDown);
		}
		break;

		case 'S':
		{
			Win32ProccessInput(input, KeyAction::MoveDown, isDown);
		}
		break;

		case VK_SPACE:
		{
			Win32ProccessInput(input, KeyAction::Jump, isDown);
		}
		break;

		case VK_SHIFT:
		{
			Win32ProccessInput(input, KeyAction::Run, isDown);
		}
		break;

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
		}
		break;

		case 'K':
		{
			if (isDown)
			{
				Win32EndPlaybackInput(&state->RecordingState);
			}
		}
		break;

		case VK_RETURN:
		{
			Win32ProccessInput(input, KeyAction::Start, isDown);
			if (altDown && isDown)
			{
				ToggleFullScreen(state, message.hwnd, &state->PrevWP);
			}
		}
		break;

		case 'P':
		{
			if (isDown && !state->RecordingState.InputRecordingIndex)
			{
				Win32BeginPlaybackInput(&state->RecordingState);
			}
		}
		break;

		default:
		{
		}
		break;
		}
}
internal void Win32ProccessInput(GameInput *input, KeyAction action, b32 isPressed)
{
	u32 actionCode = (int)action;
	if (input->States[actionCode].EndedDown != isPressed)
	{
		input->States[actionCode].EndedDown = isPressed;
		++input->States[actionCode].HalfTransitionCount;
	}
}
internal void Win32GetMousePosition(HWND windowHandle, MouseInput *mouse)
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
internal void Win32GetMouseButtonsState(GameInput *input)
{
	Win32ProccessInput(input, KeyAction::X, GetKeyState(VK_LBUTTON) & (1 << 15));
	Win32ProccessInput(input, KeyAction::B, GetKeyState(VK_RBUTTON) & (1 << 15));
}

// Graphics
internal WindowDimensions GetWindowDimensions(HWND windowHandle)
{
	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	return {width, height};
}
internal void Win32ResizeDIBSection(Win32BitmapBuffer *bitmapBuffer, i32 width, i32 height)
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
internal void Win32DisplayBufferInWindow(Win32BitmapBuffer *bitmapBuffer, HDC deviceContext, Win32ProgramState *state)
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
			SRCCOPY);
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
			SRCCOPY);
	}
}
internal void Win32DrawBuffer(const HWND &windowHandle, Win32BitmapBuffer *buffer, Win32ProgramState *state)
{
	HDC deviceContext = GetDC(windowHandle);
	Win32DisplayBufferInWindow(buffer, deviceContext, state);
	ReleaseDC(windowHandle, deviceContext);
}

// Recording
internal void Win32BeginRecordingInput(Win32RecordState *state)
{
	state->InputRecordingIndex = 1;
	const char *fileName = "recordingState.lrs";
	state->RecordingFileHandle = CreateFileA(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	DWORD bytesToWrite = (DWORD)state->TotalMemorySize;
	Assert(bytesToWrite == state->TotalMemorySize);
	DWORD bytesWritten;
	WriteFile(state->RecordingFileHandle, state->GameMemory, bytesToWrite, &bytesWritten, 0);
}
internal void Win32EndRecordingInput(Win32RecordState *state)
{
	state->InputRecordingIndex = 0;
	CloseHandle(state->RecordingFileHandle);
}
internal BOOL Win32BeginPlaybackInput(Win32RecordState *state)
{
	state->InputPlayingIndex = 1;
	const char *fileName = "recordingState.lrs";
	state->PlaybackFileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	DWORD bytesToRead = (DWORD)state->TotalMemorySize;
	Assert(bytesToRead == state->TotalMemorySize);
	DWORD bytesRead;
	return ReadFile(state->PlaybackFileHandle, state->GameMemory, bytesToRead, &bytesRead, 0);
}
internal void Win32EndPlaybackInput(Win32RecordState *state)
{
	state->InputPlayingIndex = 0;
	CloseHandle(state->PlaybackFileHandle);
}
internal void Win32RecordInput(Win32RecordState *state, GameInput *input)
{
	DWORD bytesWritten;
	WriteFile(state->RecordingFileHandle, input, sizeof(*input), &bytesWritten, 0);
}
internal BOOL Win32PlaybackInput(Win32RecordState *state, GameInput *input)
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

	Win32ProgramState *state = GetAppState(windowHandle);

	switch (message)
	{
	case WM_CREATE:
	{
		CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
		state = (Win32ProgramState *)pCreate->lpCreateParams;
		SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)state);
	}
	break;

	case WM_CLOSE:
	{
		state->IsRunning = false;
	}
	break;

	case WM_DESTROY:
	{
		state->IsRunning = false;
	}
	break;

	case WM_SIZE:
	{
	}
	break;

	case WM_ACTIVATEAPP:
	{
	}
	break;

	case WM_SETCURSOR:
	{
		if (state->ShowCursor)
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}
		else
		{
			SetCursor(NULL);
		}
	}
	break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		u32 vkCode = (u32)wParam;

		b32 wasDown = ((lParam & (1 << 30)) != 0);
		b32 isDown = ((lParam & ((u32)1 << 31)) == 0);
		b32 altDown = ((lParam & (1 << 29)) != 0);

		switch (vkCode)
		{
		case VK_F4:
		{
			// Is alt button held down
			if (altDown)
				state->IsRunning = false;
		}
		break;

		default:
		{
		}
		break;
		}
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC deviceContext = BeginPaint(windowHandle, &paint);
		Win32DisplayBufferInWindow(&state->BitmapBuffer, deviceContext, state);
		EndPaint(windowHandle, &paint);
	}
	break;

	default:
	{
		result = DefWindowProc(windowHandle, message, wParam, lParam);
	}
	break;
	}

	return result;
}

internal void Win32GetExeFileName(Win32ProgramState *State)
{
	DWORD SizeOfFilename = GetModuleFileNameA(0, State->ExeFileName, sizeof(State->ExeFileName));

	State->OnePastLastEXEFileNameSlash = State->ExeFileName;

	for (char *Scan = State->ExeFileName; *Scan; ++Scan)
	{
		if (*Scan == '\\')
		{
			State->OnePastLastEXEFileNameSlash = Scan + 1;
		}
	}
}

internal void ConCatStrings(
	size_t sourceACount, const char *sourceA,
	size_t sourceBCount, const char *sourceB,
	size_t destCount, char *dest)
{
	Assert(sourceACount + sourceBCount < destCount);

	for (size_t index = 0; index < sourceACount; ++index)
	{
		*dest++ = *sourceA++;
	}

	for (size_t index = 0; index < sourceBCount; ++index)
	{
		*dest++ = *sourceB++;
	}

	*dest++ = 0;
}

internal int StringLength(const char *string)
{
	int count = 0;

	while (*string++)
	{
		++count;
	}

	return count;
}

internal void ToggleFullScreen(Win32ProgramState *state, HWND handle, WINDOWPLACEMENT *prevWP)
{
	DWORD style = GetWindowLong(handle, GWL_STYLE);

	state->IsFullScreen = style & WS_OVERLAPPEDWINDOW;

	if (state->IsFullScreen)
	{

		MONITORINFO monitorInfo = {sizeof(monitorInfo)};

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

internal void Win32BuildEXEPathFileName(Win32ProgramState *state, const char *fileName, int destCount, char *dest)
{
	ConCatStrings(state->OnePastLastEXEFileNameSlash - state->ExeFileName, state->ExeFileName,
				  StringLength(fileName), fileName,
				  destCount, dest);
}