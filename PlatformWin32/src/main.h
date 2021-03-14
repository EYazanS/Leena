#pragma once

#include <Leena.h>

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>

struct Win32RecordState
{
	HANDLE RecordingFileHandle;
	bool32 InputRecordingIndex;
	HANDLE PlaybackFileHandle;
	bool32 InputPlayingIndex;
	uint64 TotalMemorySize;
	void* GameMemory;
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

struct Win32ProgramState
{
	bool32 IsRunning;
	Win32RecordState RecordingState;
	Win32BitmapBuffer BitmapBuffer;
	int64 PerformanceFrequence;

	char ExeFileName[MAX_PATH];
	char* OnePastLastEXEFileNameSlash;
};

#define LocalPersist static
#define GlobalVariable static
#define internal static

LRESULT CALLBACK Win32WindowCallback(HWND, UINT, WPARAM, LPARAM);

// Windows
internal inline Win32ProgramState* GetAppState(HWND handle);
internal HWND Win32InitWindow(const HINSTANCE& instance, Win32ProgramState* state);
internal MSG Win32ProcessMessage();
internal GameMemory InitGameMemory();
internal inline int64 Win32GetPerformanceFrequence();
internal inline int64 Win32GetWallClock();
internal real32 GetSecondsElapsed(uint64 start, uint64 end, uint64 frequency);
internal FILETIME GetFileLastWriteDate(const char* fileName);
internal void Win32BuildEXEPathFileName(Win32ProgramState* State, const char* FileName, int DestCount, char* Dest);
internal void Win32GetExeFileName(Win32ProgramState* State);

// Audio
internal HRESULT Wind32InitializeXAudio(IXAudio2*& xAudio);
internal HRESULT Wind32InitializeMasterVoice(IXAudio2* xAudio, IXAudio2MasteringVoice*& masteringVoice);
internal WAVEFORMATEX Wind32InitializeWaveFormat(IXAudio2* xAudio, IXAudio2SourceVoice*& sourceVoice, GameAudioBuffer* audioBuffer);
internal void Win32PlayAudio(IXAudio2SourceVoice* sourceVoice);
internal HRESULT Win32FillaudioBuffer(IXAudio2SourceVoice* sourceVoice, GameAudioBuffer* gameAudioBuffer, XAUDIO2_BUFFER& audioBuffer);

// Input
internal void Win32ProcessDigitalButton(DWORD button, DWORD buttonBit, GameButtonState* oldState, GameButtonState* newState);
internal real32 Win32ProcessXInputStickValues(real32 value, int16 deadZoneThreshold);
internal void ProccessControllerInput(GameInput* newInput, GameInput* oldInput);
internal real32 Win32CalculateTriggerValue(real32 triggerValue);
internal void ProccessKeyboardKeys(Win32ProgramState* state, MSG& message, GameControllerInput* input);
internal void Win32ProccessKeyboardMessage(GameButtonState& state, bool32 isPressed);
internal void Win32GetMousePosition(HWND windowHandle, MouseInput* mouse);
internal void Win32GetMouseButtonsState(MouseInput* mouse);

// Graphics
internal WindowDimensions GetWindowDimensions(HWND windowHandle);
internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, int width, int height);
internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext);
internal void Win32DrawBuffer(const HWND& windowHandle, Win32BitmapBuffer* buffer);

// Recording 
internal void Win32BeginRecordingInput(Win32RecordState* state);
internal void Win32EndRecordingInput(Win32RecordState* state);
internal BOOL Win32BeginPlaybackInput(Win32RecordState* state);
internal void Win32EndPlaybackInput(Win32RecordState* state);
internal void Win32RecordInput(Win32RecordState* state, GameInput* input);
internal BOOL Win32PlaybackInput(Win32RecordState* state, GameInput* input);

// File IO
bool32 DebugPlatformWriteEntireFile(ThreadContext* thread, const char* fileName, uint32 memorySize, void* memory);
void DebugPlatformFreeFileMemory(ThreadContext* thread, void* memory);
DebugFileResult DebugPlatformReadEntireFile(ThreadContext* thread, const char* fileName);


// Utilities
internal int StringLength(const char* string);
internal void ConCatStrings(
	size_t sourceACount, const char* sourceA,
	size_t sourceBCount, const char* sourceB,
	size_t destCount, char* dest);