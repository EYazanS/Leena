#pragma once
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

struct Entity
{
	bool32 Exists;

	real32 Width;
	real32 Height;

	MapPosition Position;
	Vector2d Velocity;

	uint32 FacingDirection;
};

struct GameState
{
	MemoryPool WorldMemoryPool;

	uint32 EntityIndexTheCameraIsFollowing;
	MapPosition CameraPosition;

	uint32 EntitiesCount;
	uint32 PlayersIndexesForControllers[ArrayCount(((GameInput*)0)->Controllers)];
	Entity Entities[250];

	LoadedBitmap Background;
	PlayerBitMap BitMaps[4];

	World* World;
	bool32 EnableSmoothCamera;
};


inline GameControllerInput* GetController(GameInput* input, uint8 index)
{
	Assert(index < ArrayCount(input->Controllers));
	GameControllerInput* result = &input->Controllers[index];
	return result;
}

inline Entity* GetEntity(GameState* gameState, uint32 index)
{
	Entity* result = {};

	if (index > 0 && (index < ArrayCount(gameState->Entities)))
	{
		result = &gameState->Entities[index];
	}
	return result;
}

#define LeenaH
#endif