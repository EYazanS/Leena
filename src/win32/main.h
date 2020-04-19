#pragma once

#include <Leena.h>

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <tuple>

struct ProgramState
{
	bool IsRunning;
	int64 PerformanceFrequence;
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

#define LocalPersist static
#define GlobalVariable static
#define internal static

LRESULT CALLBACK Win32WindowCallback(HWND, UINT, WPARAM, LPARAM);

// Windows
internal inline ProgramState* GetAppState(HWND handle);
internal HWND Win32InitWindow(const HINSTANCE& instance, ProgramState* state);
internal MSG Win32ProcessMessage();
internal GameMemory InitGameMemory();
internal inline int64 Win32GetPerformanceFrequence();
internal inline int64 Win32GetWallClock();
internal real32 GetSecondsElapsed(uint64 start, uint64 end, uint64 frequency);

// Audio
internal HRESULT Wind32InitializeXAudio(IXAudio2*& xAudio);
internal HRESULT Wind32InitializeMasterVoice(IXAudio2* xAudio, IXAudio2MasteringVoice*& masteringVoice);
internal WAVEFORMATEX Wind32InitializeWaveFormat(IXAudio2* xAudio, IXAudio2SourceVoice* &sourceVoice, GameAudioBuffer* audioBuffer);
internal void Win32PlaySound(IXAudio2SourceVoice* sourceVoice);
internal HRESULT Win32FillaudioBuffer(IXAudio2SourceVoice* sourceVoice, GameAudioBuffer* gameAudioBuffer, XAUDIO2_BUFFER& audioBuffer);

// Input
internal void Win32ProcessDigitalButton(DWORD button, DWORD buttonBit, GameButtonState* oldState, GameButtonState* newState);
internal real32 Win32ProcessXInputStickValues(real32 value, int16 deadZoneThreshold);
internal void ProccessControllerInput(GameInput* newInput, GameInput* oldInput);
internal real32 Win32CalculateTriggerValue(real32 triggerValue);
internal void ProccessKeyboardKeys(MSG& message, GameControllerInput* input);
internal void Win32ProccessKeyboardMessage(GameButtonState& state, bool isPressed);

// Graphics
internal std::tuple<int, int> GetWindowDimensions(HWND windowHandle);
internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int width, int height);
internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, int width, int height);
internal void Win32DrawBuffer(const HWND& windowHandle);