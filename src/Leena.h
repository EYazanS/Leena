#pragma once

#if !defined(LeenaH)

#include <math.h>
#include <map>

#include "GameTypes.h"
#include "GameStructs.h"

// Functions provided for the platform layer
void GameUpdate(GameScreenBuffer* gameScreenBuffer, GameSoundBuffer* soundBuffer, GameInput* input);

#define LeenaH

#endif