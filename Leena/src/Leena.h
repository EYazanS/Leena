#if !defined(LeenaH)


#include "GameTypes.h"
#include "GameStructs.h"
#include "RandomNumbers.h"
#include "Memory/Memory.h"

#if defined(_MSC_VER)
//  Microsoft 
#define DllExport extern "C" __declspec(dllexport)
#define DllImport __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define DllExport extern "C" __attribute__((visibility("default")))
#define DllIMPORT
#else
//  do nothing and hope for the best?
#define DllEXPORT
#define DllIMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

// Functions provided for the platform layer
DllExport void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameInput* input);

#include "Utilities/Intrinsics.h"
#include "Map/TileMap.h"

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

struct GameState
{
	MemoryPool WorldMemoryPool;
	World* World;
	TileMapPosition PlayerPosition;
};
#define LeenaH

#endif