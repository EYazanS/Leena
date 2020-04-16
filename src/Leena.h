#pragma once

#if !defined(LeenaH)

#include <math.h>

#include "GameTypes.h"
#include "GameStructs.h"

// Functions provided for the platform layer
void GameUpdate(GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameSoundBuffer* soundBuffer, GameInput* input);

inline GameControllerInput* GetController(GameInput* input, uint8 index)
{
	Assert(index < ArrayCount(input->Controllers));
	GameControllerInput* result = &input->Controllers[index];
	return result;
}

// Functions provided for the game layer
#if Leena_Internal
struct DebugFileResult
{
	void* Memory;
	uint32 FileSize;
};

void DebugPlatformFreeFileMemory(void* memory);
DebugFileResult DebugPlatformReadEntireFile(const char* fileName);
bool32 DebugPlatformWriteEntireFile(const char* fileName, uint32 memorySize, void* memory);

#endif // Leena_Internal

#define LeenaH

#endif