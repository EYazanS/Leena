#include "Memory.h"

void InitilizePool(MemoryPool* pool, MemorySizeIndex size, u8* storage)
{
	pool->Size = size;
	pool->BaseMemory = storage;
	pool->UsedAmount = 0;
}

void* PushSize_(MemoryPool* pool, MemorySizeIndex size)
{
	Assert(pool->UsedAmount + pool->UsedAmount <= pool->Size);
	void* result = pool->BaseMemory + pool->UsedAmount;
	pool->UsedAmount += size;
	return result;
}