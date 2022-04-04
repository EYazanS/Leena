#include <SDL2/SDL.h>

#include "linux.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <iostream>
#include <libgen.h>

#define MAX_CONTROLLERS 4

#define LEFT_THUMB_DEADZONE 7849
#define RIGHT_THUMB_DEADZONE 8689
#define TRIGGER_THRESHOLD 30

GlobalVariable LinuxProgramState programState = {};

int StringLength(const char *string)
{
	int count = 0;

	while (*string++)
	{
		++count;
	}

	return count;
}

void ConCatStrings(
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

void LinuxGetExeFileName(LinuxProgramState *state)
{
	ssize_t count = readlink("/proc/self/exe", state->ExeFileName, PATH_MAX);

	if (count != -1)
	{
		state->OnePastLastEXEFileNameSlash = dirname(state->ExeFileName);
	}
}

void Win32BuildEXEPathFileName(LinuxProgramState *state, const char *fileName, int destCount, char *dest)
{
	ConCatStrings(
		StringLength(state->OnePastLastEXEFileNameSlash),
		state->ExeFileName,
		StringLength(fileName), fileName,
		destCount, dest);
}

#if Leena_Internal
DebugFileResult DebugPlatformReadEntireFile(ThreadContext *thread, const char *fileName)
{
	DebugFileResult result = {};

	char fullFileName[PATH_MAX];
	Win32BuildEXEPathFileName(&programState, fileName, sizeof(fullFileName), fullFileName);

	int fileHandle = open(fullFileName, O_RDONLY);

	if (fileHandle == -1)
	{
		return result;
	}

	struct stat fileStatus;

	if (fstat(fileHandle, &fileStatus) == -1)
	{
		// To get the last error
		// auto err = strerror(errno);
		close(fileHandle);
		return result;
	}

	result.FileSize = SafeTruncateUInt64(fileStatus.st_size);
	result.Memory = malloc(result.FileSize);

	if (!result.Memory)
	{
		result.FileSize = 0;
		close(fileHandle);
		return result;
	}

	u32 bytesToRead = result.FileSize;

	u8 *nextByteLocation = (u8 *)result.Memory;

	while (bytesToRead)
	{
		i32 bytesRead = read(fileHandle, nextByteLocation, bytesToRead);

		if (bytesRead == -1)
		{
			free(result.Memory);
			result.Memory = 0;
			result.FileSize = 0;
			close(fileHandle);
			return result;
		}

		bytesToRead -= bytesRead;
		nextByteLocation += bytesRead;
	}

	close(fileHandle);

	return result;
}

void DebugPlatformFreeFileMemory(ThreadContext *thread, void *memory)
{
	free(memory);
}

b32 DebugPlatformWriteEntireFile(ThreadContext *thread, const char *fileName, u32 memorySize, void *memory)
{
	i32 FileHandle = open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (FileHandle == -1)
		return false;

	u32 BytesToWrite = memorySize;

	u8 *NextByteLocation = (u8 *)memory;

	while (BytesToWrite)
	{
		i32 bytesWritten = write(FileHandle, NextByteLocation, BytesToWrite);

		if (bytesWritten == -1)
		{
			close(FileHandle);
			return false;
		}

		BytesToWrite -= bytesWritten;

		NextByteLocation += bytesWritten;
	}

	close(FileHandle);

	return true;
}
#endif

WindowDimension SDLGetWindowDimension(SDL_Window *window)
{
	WindowDimension result;

	SDL_GetWindowSize(window, &result.Width, &result.Height);

	return (result);
}

void LoadGameFunctions()
{
	// void *LibHandle = dlopen("Leena.dll", RTLD_NOW);
}

GameMemory InitGameMemory()
{
	GameMemory gameMemory = {};

#if Leena_Internal
	void *baseAddress = (void *)Terabytes(2);
#else
	void *baseAddress = 0;
#endif

	gameMemory.PermanentStorageSize = Megabytes(256);
	gameMemory.TransientStorageSize = Gigabytes(1);

	size_t totalSize = gameMemory.PermanentStorageSize + gameMemory.TransientStorageSize;

	gameMemory.PermanentStorage = mmap(
		baseAddress, totalSize,
		PROT_READ | PROT_WRITE,
		MAP_ANON | MAP_PRIVATE,
		-1, 0);

	gameMemory.TransientStorage = (u8 *)(gameMemory.PermanentStorage) + gameMemory.PermanentStorageSize;

	gameMemory.FreeFile = DebugPlatformFreeFileMemory;
	gameMemory.ReadFile = DebugPlatformReadEntireFile;
	gameMemory.WriteFile = DebugPlatformWriteEntireFile;

	return gameMemory;
}

Internal void ProcessGameControllerButton(
	ButtonState *oldState,
	ButtonState *newState,
	SDL_GameController *controllerHandle,
	SDL_GameControllerButton button)
{
	newState->EndedDown = SDL_GameControllerGetButton(controllerHandle, button);
	newState->HalfTransitionCount += ((newState->EndedDown == oldState->EndedDown) ? 0 : 1);
}
Internal r32
SDLProcessGameControllerAxisValue(i16 value, i16 deadZoneThreshold)
{
	r32 result = 0;

	if (value < -deadZoneThreshold)
	{
		result = (r32)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	}
	else if (value > deadZoneThreshold)
	{
		result = (r32)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
	}

	return (result);
}

Internal void SDLProcessKeyPress(ButtonState *newState, b32 isDown)
{
	Assert(newState->EndedDown != isDown);
	newState->EndedDown = isDown;
	++newState->HalfTransitionCount;
}

int main(int args, char *argv[])
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Failed to initialize the SDL2 library\n";
		std::cout << "Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_Window *window = SDL_CreateWindow(
		"Leena game engine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		960, 540,
		SDL_WINDOW_RESIZABLE);

	if (!window)
	{
		std::cout << "Failed to create window\n";
		return -1;
	}

	SDL_Surface *window_surface = SDL_GetWindowSurface(window);

	if (!window_surface)
	{
		std::cout << "Failed to get the surface from the window\n";
		return -1;
	}

	SDL_UpdateWindowSurface(window);

	// GameMemory gameMemory = InitGameMemory();

	LinuxGetExeFileName(&programState);

	SDL_Renderer *renderer = SDL_GetRenderer(window);

	WindowDimension windowDim = SDLGetWindowDimension(window);

	ScreenBuffer screenBuffer;

	screenBuffer.Width = windowDim.Width;
	screenBuffer.Height = windowDim.Height;

	SDL_Texture *texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		windowDim.Width,
		windowDim.Height);

	screenBuffer.Memory = mmap(
		0,
		windowDim.Width * windowDim.Height * 4,
		PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE,
		-1,
		0);

	screenBuffer.BytesPerPixel = 4;

	screenBuffer.Pitch = screenBuffer.Width * screenBuffer.BytesPerPixel;

	SDL_GameController *controllerHandles[MAX_CONTROLLERS] = {};

	int maxJoysticks = SDL_NumJoysticks();

	int controllerIndex = 0;

	for (int joystickIndex = 0; joystickIndex < maxJoysticks; ++joystickIndex)
	{
		if (!SDL_IsGameController(joystickIndex))
		{
			continue;
		}

		if (controllerIndex >= MAX_CONTROLLERS)
		{
			break;
		}

		controllerHandles[controllerIndex] = SDL_GameControllerOpen(joystickIndex);

		controllerIndex++;
	}

	u64 perfCountFrequency = SDL_GetPerformanceFrequency();

	u64 lastCounter = SDL_GetPerformanceCounter();

	u64 lastCycleCount = _rdtsc();

	GameInput input[2] = {};
	GameInput *newInput = &input[0];
	GameInput *oldInput = &input[1];

	bool isRunning = true;

	while (isRunning)
	{
		u64 endCounter = SDL_GetPerformanceCounter();
		u64 counterElapsed = endCounter - lastCounter;

		for (u32 stateIndex = 0; stateIndex < ArrayCount(newInput->Keyboard.Buttons); stateIndex++)
			newInput->Keyboard.Buttons[stateIndex].EndedDown = oldInput->Keyboard.Buttons[stateIndex].EndedDown;

		// Pull event
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				b32 wasDown = false;

				if (event.key.state == SDL_RELEASED)
				{
					wasDown = true;
				}
				else if (event.key.repeat != 0)
				{
					wasDown = true;
				}

				SDL_Keycode keyCode = event.key.keysym.sym;

				if (keyCode == SDLK_w)
				{
					SDLProcessKeyPress(&newInput->Keyboard.Up, wasDown);
				}

				bool AltKeyWasDown = (event.key.keysym.mod & KMOD_ALT);

				// We can then just check if the AltKeyWasDown and F4 was pressed, we quit:
				if (keyCode == SDLK_F4 && AltKeyWasDown)
				{
					isRunning = false;
				}
			}
			break;
			case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
				{
				}
				break;
				}
			}
			break;

			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMAPPED:
			case SDL_CONTROLLERDEVICEREMOVED:
			{
				for (int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; ++controllerIndex)
				{
					if (controllerHandles[controllerIndex])
					{
						SDL_GameControllerClose(controllerHandles[controllerIndex]);
					}
				}
			}
			break;

			case SDL_QUIT:
				isRunning = false;
			}
		}

		for (u32 controllerIndex = 0;
			 controllerIndex < MAX_CONTROLLERS;
			 ++controllerIndex)
		{
			SDL_GameController *controller = controllerHandles[controllerIndex];

			if (!controller || !SDL_GameControllerGetAttached(controller))
			{
				// TODO: This controller is note plugged in.
				continue;
			}

			// NOTE: We have a controller with index ControllerIndex.
			newInput->Controller.IsAnalog = true;

			SDL_GameController *sdlCOntroller = controllerHandles[controllerIndex];

			ProcessGameControllerButton(
				&(oldInput->Controller.DpadUp),
				&(newInput->Controller.DpadUp),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_DPAD_UP);

			ProcessGameControllerButton(
				&(oldInput->Controller.DpadDown),
				&(newInput->Controller.DpadDown),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_DPAD_DOWN);

			ProcessGameControllerButton(
				&(oldInput->Controller.DpadLeft),
				&(newInput->Controller.DpadLeft),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_DPAD_LEFT);

			ProcessGameControllerButton(
				&(oldInput->Controller.DpadRight),
				&(newInput->Controller.DpadRight),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

			ProcessGameControllerButton(
				&(oldInput->Controller.Start),
				&(newInput->Controller.Start),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_START);

			ProcessGameControllerButton(
				&(oldInput->Controller.Select),
				&(newInput->Controller.Select),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_BACK);

			ProcessGameControllerButton(
				&(oldInput->Controller.LeftShoulder),
				&(newInput->Controller.LeftShoulder),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_LEFTSHOULDER);

			ProcessGameControllerButton(
				&(oldInput->Controller.RightShoulder),
				&(newInput->Controller.RightShoulder),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

			ProcessGameControllerButton(
				&(oldInput->Controller.A),
				&(newInput->Controller.A),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_A);

			ProcessGameControllerButton(
				&(oldInput->Controller.B),
				&(newInput->Controller.B),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_B);

			ProcessGameControllerButton(
				&(oldInput->Controller.X),
				&(newInput->Controller.X),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_X);

			ProcessGameControllerButton(
				&(oldInput->Controller.Y),
				&(newInput->Controller.Y),
				sdlCOntroller,
				SDL_CONTROLLER_BUTTON_Y);

			i16 StickY = SDL_GameControllerGetAxis(controllerHandles[controllerIndex], SDL_CONTROLLER_AXIS_LEFTY);
			i16 StickX = SDL_GameControllerGetAxis(controllerHandles[controllerIndex], SDL_CONTROLLER_AXIS_LEFTX);

			newInput->Controller.LeftStickAverageX = SDLProcessGameControllerAxisValue(StickX, LEFT_THUMB_DEADZONE);
			newInput->Controller.LeftStickAverageY = SDLProcessGameControllerAxisValue(StickY, RIGHT_THUMB_DEADZONE);
		}

		// GAME CODE

		// Update Window
		if (SDL_UpdateTexture(
				texture,
				0,
				screenBuffer.Memory,
				screenBuffer.Pitch))
		{
			// TODO: Do something about this error!
		}

		SDL_RenderCopy(
			renderer,
			texture,
			0,
			0);

		// draw to the window
		SDL_RenderPresent(renderer);

		GameInput *temp = newInput;
		newInput = oldInput;
		oldInput = temp;

		r64 MSPerFrame = (((1000.0f * (r64)counterElapsed) / (r64)perfCountFrequency));
		r64 FPS = (r64)perfCountFrequency / (r64)counterElapsed;

		u64 endCycleCount = _rdtsc();
		u64 cyclesElapsed = endCycleCount - lastCycleCount;
		r64 MCPF = ((r64)cyclesElapsed / (1000.0f * 1000.0f));

		printf("%.02fms/f, %.02f/s, %.02fmc/f\n", MSPerFrame, FPS, MCPF);

		lastCounter = endCounter;
	}

	if (screenBuffer.Memory)
	{
		munmap(screenBuffer.Memory, windowDim.Height * screenBuffer.Pitch);
	}

	if (texture)
	{
		SDL_DestroyTexture(texture);
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}