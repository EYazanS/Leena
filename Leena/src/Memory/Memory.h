#pragma once

#include "GameTypes.h"

struct MemoryPool
{
	MemorySizeIndex Size;
	MemorySizeIndex UsedAmount;
	u8* BaseMemory;
};

void* PushSize_(MemoryPool* pool, MemorySizeIndex size);
void InitilizePool(MemoryPool* pool, MemorySizeIndex size, u8* storage);

#define PushArray(pool, size, type) (type*) PushSize_(pool, size +  sizeof(type))
#define PushSize(pool, type) (type*) PushSize_(pool, sizeof(type))