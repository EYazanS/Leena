#include "main.h"

#if Leena_Internal
DebugFileResult DebugPlatformReadEntireFile(ThreadContext* thread, const char* fileName)
{
	DebugFileResult result = {};

	HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;

		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			Assert(fileSize.QuadPart < 0xFFFFFFFF);

			u32 fileSize32 = (u32)fileSize.QuadPart;

			void* memResult = VirtualAlloc(NULL, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			if (memResult)
			{
				DWORD bytesRead;

				if (ReadFile(fileHandle, memResult, fileSize32, &bytesRead, NULL))
				{
					result.FileSize = fileSize32;
					result.Memory = memResult;
				}
				else
				{
					// Free memory if read fail
					DebugPlatformFreeFileMemory(thread, memResult);
					memResult = 0;
				}
			}
		}

		CloseHandle(fileHandle);
	}

	return result;
}

void DebugPlatformFreeFileMemory(ThreadContext* thread, void* memory)
{
	if (memory)
		VirtualFree(memory, NULL, MEM_RELEASE);
}

b32 DebugPlatformWriteEntireFile(ThreadContext* thread, const char* fileName, u32 memorySize, void* memory)
{
	b32 result = 0;
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;

		if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, NULL))
		{
			result = bytesWritten == memorySize;
		}

		CloseHandle(fileHandle);
	}

	return result;
}
#endif