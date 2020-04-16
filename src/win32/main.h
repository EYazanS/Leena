#pragma once

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

LRESULT CALLBACK Win32WindowCallback(HWND, UINT, WPARAM, LPARAM);

// Windows
internal inline ProgramState* GetAppState(HWND handle);
internal HWND Win32InitWindow(const HINSTANCE& instance, ProgramState* state);
internal MSG Win32ProcessMessage();
internal int64 Win32GetPerformanceFrequence();
internal int64 Win32QueryPerformance();

// Audio
internal HRESULT Wind32InitializeXAudio(IXAudio2* xAudio, Wind32SoundBuffer* soundBuffer);
internal HRESULT Win32FillSoundBuffer(IXAudio2SourceVoice* sourceVoice, GameSoundBuffer* soundBuffer);
internal void Win32PlaySound(IXAudio2SourceVoice* sourceVoice);
internal Wind32SoundBuffer IniWin32SoundBuffer();

// Input
internal void Win32ProcessDigitalButton(DWORD button, DWORD buttonBit, GameButtonState* oldState, GameButtonState* newState);
internal real32 Win32ProcessXInputStickValues(real32 value, int16 deadZoneThreshold);
internal GameMemory InitGameMemory();
internal void ProccessControllerInput(GameInput* newInput, GameInput* oldInput);
internal void ProccessKeyboardKeys(MSG& message, GameControllerInput* input);
internal void Win32ProccessKeyboardMessage(GameButtonState& state, bool isPressed);

// Graphics
internal std::tuple<int, int> GetWindowDimensions(HWND windowHandle);
internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int width, int height);
internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, int width, int height);
internal void Win32DrawBuffer(const HWND& windowHandle);