#pragma once

#if !defined(LeenaH)

#include "GameTypes.h"
#include "Math/Math.h"
#include "GameStructs.h"
#include "RandomNumbers.h"
#include "Memory/Memory.h"
#include "Utilities/Intrinsics.h"
#include "Map/TileMap.h"

// Functions provided for the platform layer
DllExport void GameUpdateAndRender(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameInput* input);
DllExport void GameUpdateAudio(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer* audioBuffer);

inline GameControllerInput* GetController(GameInput* input, uint8 index)
{
	Assert(index < ArrayCount(input->Controllers));
	GameControllerInput* result = &input->Controllers[index];
	return result;
}

struct World
{
	Map* Map;
};

struct LoadedBitmap
{
	int64 Width;
	int64 Height;
	uint32* Data;
};

struct PlayerBitMap
{
	LoadedBitmap Head;
	LoadedBitmap Torso;
	LoadedBitmap Cape;

	int32 AlignX;
	int32 AlignY;
};

struct GameState
{
	MemoryPool WorldMemoryPool;
	
	MapPosition CameraPosition;
	MapPosition PlayerPosition;

	Vector2d PlayerVelocity;

	LoadedBitmap Background;
	PlayerBitMap playerBitMaps[4];
	uint32 PlayerFacingDirection;
	
	World* World;
	bool32 EnableSmoothCamera;
};
#define LeenaH

#endif