#pragma once

#if !defined(LeenaH)

#include <math.h>

#include "GameTypes.h"
#include "GameStructs.h"

#define DllExport extern "C" __declspec(dllexport)

// Functions provided for the platform layer
DllExport void GameUpdate(GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameAudioBuffer* soundBuffer, GameInput* input);

inline GameControllerInput* GetController(GameInput* input, uint8 index)
{
	Assert(index < ArrayCount(input->Controllers));
	GameControllerInput* result = &input->Controllers[index];
	return result;
}
#define LeenaH

#endif