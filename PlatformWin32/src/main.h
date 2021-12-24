#pragma once

#include <Leena.h>

#include <Windows.h>
#include <Xinput.h>
#include <xaudio2.h>

struct Win32RecordState
{
	HANDLE RecordingFileHandle;
	b32 InputRecordingIndex;
	HANDLE PlaybackFileHandle;
	b32 InputPlayingIndex;
	u64 TotalMemorySize;
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
	b32 IsRunning;
	b32 ShowCursor;
	b32 IsFullScreen;
	i32 WindowWidth;
	i32 WindowHeight;
	Win32RecordState RecordingState;
	Win32BitmapBuffer BitmapBuffer;
	WINDOWPLACEMENT PrevWP;

	i64 PerformanceFrequence;

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
internal inline i64 Win32GetPerformanceFrequence();
internal inline i64 Win32GetWallClock();
internal r32 GetSecondsElapsed(u64 start, u64 end, u64 frequency);
internal FILETIME GetFileLastWriteDate(const char* fileName);
internal void Win32BuildEXEPathFileName(Win32ProgramState* State, const char* FileName, int DestCount, char* Dest);
internal void Win32GetExeFileName(Win32ProgramState* State);
internal void ToggleFullScreen(Win32ProgramState* state, HWND handle, WINDOWPLACEMENT* prevWP);

// Audio
internal HRESULT Wind32InitializeXAudio(IXAudio2*& xAudio);
internal HRESULT Wind32InitializeMasterVoice(IXAudio2* xAudio, IXAudio2MasteringVoice*& masteringVoice);
internal WAVEFORMATEX Wind32InitializeWaveFormat(IXAudio2* xAudio, IXAudio2SourceVoice*& sourceVoice, AudioBuffer* audioBuffer);
internal void Win32PlayAudio(IXAudio2SourceVoice* sourceVoice);
internal HRESULT Win32FillaudioBuffer(IXAudio2SourceVoice* sourceVoice, AudioBuffer* gameAudioBuffer, XAUDIO2_BUFFER& audioBuffer);

// Input
internal r32 Win32ProcessXInputStickValues(r32 value, i16 deadZoneThreshold);
internal void ProccessControllerInput(GameInput* newInput, GameInput* oldInput);
internal r32 Win32CalculateTriggerValue(r32 triggerValue);
internal void ProccessKeyboardKeys(Win32ProgramState* state, MSG& message, GameInput* input);
internal void Win32GetMousePosition(HWND windowHandle, MouseInput* mouse);
internal void Win32GetMouseButtonsState(GameInput* mouse);
internal void Win32ProccessInput(GameInput* keyboard, KeyAction action, b32 isPressed);

// Graphics
internal WindowDimensions GetWindowDimensions(HWND windowHandle);
internal void Win32ResizeDIBSection(Win32BitmapBuffer* bitmapBuffer, i32 width, i32 height);
internal void Win32DisplayBufferInWindow(Win32BitmapBuffer* bitmapBuffer, HDC deviceContext, Win32ProgramState* state);
internal void Win32DrawBuffer(const HWND& windowHandle, Win32BitmapBuffer* buffer, Win32ProgramState* state);

// Recording 
internal void Win32BeginRecordingInput(Win32RecordState* state);
internal void Win32EndRecordingInput(Win32RecordState* state);
internal BOOL Win32BeginPlaybackInput(Win32RecordState* state);
internal void Win32EndPlaybackInput(Win32RecordState* state);
internal void Win32RecordInput(Win32RecordState* state, GameInput* input);
internal BOOL Win32PlaybackInput(Win32RecordState* state, GameInput* input);

// File IO
b32 DebugPlatformWriteEntireFile(ThreadContext* thread, const char* fileName, u32 memorySize, void* memory);
void DebugPlatformFreeFileMemory(ThreadContext* thread, void* memory);
DebugFileResult DebugPlatformReadEntireFile(ThreadContext* thread, const char* fileName);


// Utilities
internal int StringLength(const char* string);
internal void ConCatStrings(
	size_t sourceACount, const char* sourceA,
	size_t sourceBCount, const char* sourceB,
	size_t destCount, char* dest);