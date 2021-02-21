#if !defined(LeenaH)

#include "GameTypes.h"
#include "GameStructs.h"
#include "Utilities/Intrinsics.h"

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
DllExport void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* gameScreenBuffer, GameAudioBuffer* soundBuffer, GameInput* input);

inline GameControllerInput* GetController(GameInput* input, uint8 index)
{
	Assert(index < ArrayCount(input->Controllers));
	GameControllerInput* result = &input->Controllers[index];
	return result;
}

struct GameState
{
	WorldPosition PlayerPosition;
};

struct TileChunk
{
	uint32* Tiles;
};

struct World
{
	uint32 ChunkDimension;

	real32 TileSideInMeters;
	
	int32 TileSideInPixels;

	real32 MetersToPixels;
	
	uint32 TileChunkCountX;
	uint32 TileChunkCountY;
	
	uint32 ChunkShift;
	uint32 ChunkMask;

	TileChunk* TileChunks;
};

#define LeenaH

#endif