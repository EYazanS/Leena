#include "Leena.h"

void RenderPlayer(GameScreenBuffer* gameScreenBuffer, World* world, GameState* gameState, real32 playerX, real32 playerY, real32 playerWidth, real32 playerHeight);
void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer);
GameAudioBuffer* ReadAudioBufferData(void* memory);

void DrawRectangle(
	GameScreenBuffer* gameScreenBuffer,
	real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY,
	Colour colour);
void DrawTileMap(World* world, GameState* gameState, GameScreenBuffer* screenBuffer, TileMap* tileMap);
int32 IsTileMapPointEmpty(World* world, TileMap* tileMap, uint32 tileTestX, uint32 tileTestY);
bool32 IsWorldPointEmpty(World* world, CononicalLocation location);
inline int32 GetTileValueUnchecked(TileMap* tileMap, size_t tileCountX, size_t tileX, size_t tileY);
inline TileMap* GetTileMap(World* world, size_t tileX, size_t tileY);
inline CononicalLocation RecanonicalìzeLocation(World* world, CononicalLocation location);
inline void RecanonicalizeCoord(World* world, int32 tileCount, uint32* tileMap, int32* tile, real32* tileRelative);
inline real32 MetersToPixels(World* world, int32 meters);
inline real32 MetersToPixels(World* world, real32 meters);

void GameUpdate(ThreadContext* thread, GameMemory* gameMemory, GameScreenBuffer* screenBuffer, GameAudioBuffer* soundBuffer, GameInput* input)
{
	GameState* gameState = (GameState*)gameMemory->PermenantStorage;

	if (!gameMemory->IsInitialized)
	{
		gameState->PlayerLocation.TileMapX = 0;
		gameState->PlayerLocation.TileMapX = 0;

		gameState->PlayerLocation.TileX = 3;
		gameState->PlayerLocation.TileY = 3;

		gameState->PlayerLocation.TileRelativeX = 0;
		gameState->PlayerLocation.TileRelativeY = 0;

		gameMemory->IsInitialized = true;
	}

	TileMap tileMaps[2][2] = {};

	World world = {};

	world.TileMapCountX = 2;
	world.TileMapCountY = 2;

	world.TileMaps = (TileMap*)&tileMaps;
	world.CountX = 16;
	world.CountY = 9;

	world.TileSideInMeters = 1.4f;
	world.TileSideInPixels = 60;

	world.MetersToPixels = world.TileSideInPixels / world.TileSideInMeters;

	world.UpperLeftX = -world.TileSideInPixels / 2;
	world.UpperLeftY = 0;

	uint32 tiles00[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles01[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles10[16][9] =
	{
		1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	uint32 tiles11[16][9] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	tileMaps[0][0].Tiles = (uint32*)tiles00;
	tileMaps[0][1].Tiles = (uint32*)tiles01;
	tileMaps[1][0].Tiles = (uint32*)tiles10;
	tileMaps[1][1].Tiles = (uint32*)tiles11;

	TileMap* currentTileMap = GetTileMap(&world, gameState->PlayerLocation.TileMapX, gameState->PlayerLocation.TileMapY);
	Assert(currentTileMap);

	// In Meters
	real32 playerHeight = 1.25f;
	real32 playerWidth = 0.75f * playerHeight;

	real32 playerMovementX = 0.f; // pixels/second
	real32 playerMovementY = 0.f; // pixels/second

	if (input->Keyboard.A.EndedDown)
		playerMovementX += -1.f;

	if (input->Keyboard.D.EndedDown)
		playerMovementX += 1.f;

	if (input->Keyboard.W.EndedDown)
		playerMovementY += -1.f;

	if (input->Keyboard.S.EndedDown)
		playerMovementY += 1.f;

	playerMovementY *= 5;
	playerMovementX *= 5;

	//// TODO: Deal with controller movement
	//for (const GameControllerInput& controller : input->Controllers)
	//{
	//	if (controller.IsConnected && controller.IsAnalog)
	//	{
	//		gameState->PlayerLocation.TileRelativeX += static_cast<int>(pixelsToMovePerSec * input->TimeToAdvance * controller.LeftStickAverageX);
	//		gameState->PlayerLocation.TileRelativeY += static_cast<int>(pixelsToMovePerSec * input->TimeToAdvance * controller.LeftStickAverageY);
	//	}
	//}

	CononicalLocation newPlayerLocation = gameState->PlayerLocation;

	newPlayerLocation.TileRelativeX += (real32)(input->TimeToAdvance * playerMovementX);
	newPlayerLocation.TileRelativeY += (real32)(input->TimeToAdvance * playerMovementY);

	newPlayerLocation = RecanonicalìzeLocation(&world, newPlayerLocation);

	CononicalLocation playerLeftLocation = newPlayerLocation;
	playerLeftLocation.TileRelativeX -= 0.5f * playerWidth;
	playerLeftLocation = RecanonicalìzeLocation(&world, playerLeftLocation);

	CononicalLocation playerRightLocation = newPlayerLocation;
	playerRightLocation.TileRelativeX += 0.5f * playerWidth;
	playerRightLocation = RecanonicalìzeLocation(&world, playerRightLocation);

	if (IsWorldPointEmpty(&world, newPlayerLocation) && IsWorldPointEmpty(&world, playerLeftLocation) && IsWorldPointEmpty(&world, playerRightLocation))
		gameState->PlayerLocation = newPlayerLocation;

	DrawTileMap(&world, gameState, screenBuffer, currentTileMap);

	RenderPlayer(screenBuffer, &world, gameState, MetersToPixels(&world, gameState->PlayerLocation.TileRelativeX), MetersToPixels(&world, gameState->PlayerLocation.TileRelativeY), MetersToPixels(&world, playerWidth), MetersToPixels(&world, playerHeight));

	FillAudioBuffer(thread, gameMemory, soundBuffer);
}

inline void RecanonicalizeCoord(World* world, int32 tileCount, uint32* tileMap, int32* tile, real32* tileRelative)
{
	int32 offset = FloorReal32ToInt32((*tileRelative) / world->TileSideInMeters);
	*tile += offset;
	*tileRelative -= offset * world->TileSideInMeters;

	Assert(*tileRelative >= 0);
	Assert(*tileRelative < world->TileSideInMeters);

	if (*tile < 0)
	{
		*tile = tileCount + *tile;
		--(*tileMap);
	}
	else if (*tile >= tileCount)
	{
		*tile = *tile - tileCount;
		++(*tileMap);
	}
}

CononicalLocation RecanonicalìzeLocation(World* world, CononicalLocation location)
{
	CononicalLocation result = location;

	RecanonicalizeCoord(world, world->CountX, &result.TileMapX, &result.TileX, &result.TileRelativeX);
	RecanonicalizeCoord(world, world->CountY, &result.TileMapY, &result.TileY, &result.TileRelativeY);

	return result;
}

bool32 IsWorldPointEmpty(World* world, CononicalLocation location)
{
	int32 isEmpty = 0;

	TileMap* tileMap = GetTileMap(world, location.TileMapX, location.TileMapY);

	isEmpty = IsTileMapPointEmpty(world, tileMap, location.TileX, location.TileY);

	return isEmpty;
}

int32 IsTileMapPointEmpty(World* world, TileMap* tileMap, uint32 testTileX, uint32 testTileY)
{
	int32 isEmpty = 0;

	if (tileMap)
	{
		if ((testTileX >= 0 && testTileX < world->CountX) && (testTileY >= 0 && testTileY < world->CountY))
		{
			uint32 tileMapValue = GetTileValueUnchecked(tileMap, world->CountX, testTileX, testTileY);
			isEmpty = tileMapValue == 0;
		}
	}

	return isEmpty;
}

void DrawTileMap(World* world, GameState* gameState, GameScreenBuffer* screenBuffer, TileMap* tileMap)
{
	for (size_t row = 0; row < world->CountY; row++)
	{
		for (size_t column = 0; column < world->CountX; column++)
		{
			Colour colour = {};

			(GetTileValueUnchecked(tileMap, world->CountX, column, row) == 1) ? colour = { 1.f, 1.f, 1.f } : colour = { 0.7f, 0.7f, 0.7f };

			// For debug
			if (column == gameState->PlayerLocation.TileX && row == gameState->PlayerLocation.TileY)
				colour = { 0.0f, 0.0f ,0.0f };

			real32 minX = world->UpperLeftX + ((real32)column * world->TileSideInPixels);
			real32 maxX = minX + world->TileSideInPixels;

			real32 minY = world->UpperLeftY + ((real32)row * world->TileSideInPixels);
			real32 maxY = minY + world->TileSideInPixels;

			DrawRectangle(screenBuffer, minX, minY, maxX, maxY, colour);
		}
	}
}

void FillAudioBuffer(ThreadContext* thread, GameMemory* gameMemory, GameAudioBuffer*& soundBuffer)
{
	if (0)
	{
		DebugFileResult file = gameMemory->ReadFile(thread, "resources/Water_Splash_SeaLion_Fienup_001.wav");

		auto result = ReadAudioBufferData(file.Memory);

		*soundBuffer = *result;
	}
}

GameAudioBuffer* ReadAudioBufferData(void* memory)
{
	uint8* byte = (uint8*)memory; // Get the firs 4 bytes of the memory

	// Move to position 21
	byte += 20;

	GameAudioBuffer* result = (GameAudioBuffer*)byte;

	// Move to Data chunk
	char* characterToRead = ((char*)byte);

	characterToRead += sizeof(GameAudioBuffer);

	while (*characterToRead != 'd' || *(characterToRead + 1) != 'a' || *(characterToRead + 2) != 't' || *(characterToRead + 3) != 'a')
	{
		characterToRead++;
	}

	characterToRead += 4;

	byte = (uint8*)(characterToRead);

	uint32 bufferSize = *((uint32*)byte);

	byte += 4;

	// Get one third of a sec
	uint32 totalBytesNeeded = (uint32)(result->SamplesPerSec * 0.5f);

	result->BufferSize = totalBytesNeeded;
	result->BufferData = byte;

	return result;
}

void RenderPlayer(GameScreenBuffer* gameScreenBuffer, World* world, GameState* gameState, real32 playerX, real32 playerY, real32 playerWidth, real32 playerHeight)
{
	Colour colour = { 1.f, 0.f, 1.f };

	real32 playerLeft = world->UpperLeftX + (world->TileSideInPixels * gameState->PlayerLocation.TileX) + playerX - (0.5f * playerWidth);
	real32 playerTop = world->UpperLeftY + (world->TileSideInPixels * gameState->PlayerLocation.TileY) + playerY - playerHeight;

	DrawRectangle(gameScreenBuffer, playerLeft, playerTop, playerLeft + playerWidth, playerTop + playerHeight, colour);
}

void DrawRectangle(
	GameScreenBuffer* gameScreenBuffer,
	real32 realMinX, real32 realMinY, real32 realMaxX, real32 realMaxY,
	Colour colour)
{
	int32 minX = RoundReal32ToInt32(realMinX);
	int32 maxX = RoundReal32ToInt32(realMaxX);

	int32 minY = RoundReal32ToInt32(realMinY);
	int32 maxY = RoundReal32ToInt32(realMaxY);

	// Clip to the nearest valid pixel
	if (minX < 0)
		minX = 0;

	if (minY < 0)
		minY = 0;

	if (maxX > gameScreenBuffer->Width)
		maxX = gameScreenBuffer->Width;

	if (maxY > gameScreenBuffer->Height)
		maxY = gameScreenBuffer->Height;

	// Bit Pattern = x0 AA RR GG BB
	uint32 finalColour =
		(RoundReal32ToInt32(colour.Red * 255.f) << 16) |
		(RoundReal32ToInt32(colour.Green * 255.f) << 8) |
		(RoundReal32ToInt32(colour.Blue * 255.f) << 0);

	uint8* endOfBuffer = ((uint8*)gameScreenBuffer->Memory) + ((uint64)gameScreenBuffer->Pitch * (uint64)gameScreenBuffer->Height);

	uint8* row = (((uint8*)gameScreenBuffer->Memory) + (int64)minX * 4 + (int64)minY * gameScreenBuffer->Pitch);

	for (int32 y = minY; y < maxY; ++y)
	{
		uint32* pixel = (uint32*)row;

		for (int32 x = minX; x < maxX + 10; ++x)
		{
			*pixel++ = finalColour;
		}

		row += gameScreenBuffer->Pitch;
	}
}

inline int32 GetTileValueUnchecked(TileMap* tileMap, size_t tileCountX, size_t tileX, size_t tileY)
{
	return tileMap->Tiles[tileY * tileCountX + tileX];
}

inline TileMap* GetTileMap(World* world, size_t tileX, size_t tileY)
{
	TileMap* tileMap = 0;

	if ((tileX >= 0 && tileX <= world->TileMapCountX) && (tileY >= 0 && tileY <= world->TileMapCountY))
		tileMap = &world->TileMaps[tileY * world->TileMapCountX + tileX];

	return tileMap;
}

inline real32 MetersToPixels(World* world, int32 meters)
{
	return world->MetersToPixels * meters;
}

inline real32 MetersToPixels(World* world, real32 meters)
{
	return world->MetersToPixels * meters;
}