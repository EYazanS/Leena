#pragma once

#include <math.h>

#if !defined(LeenaH)

#include "Defines.h"
#include "GameStructs.h"

// Functions provided for the platform layer
void GameUpdate(GameScreenBuffer* gameScreenBuffer, GameSoundBuffer* soundBuffer);

#define LeenaH
	
#endif