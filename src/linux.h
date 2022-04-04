#pragma once

#include "Leena.h"

#include <linux/limits.h>

#define LocalPersist static
#define GlobalVariable static
#define Internal static

struct LinuxProgramState
{
	b32 IsRunning;
	b32 ShowCursor;
	b32 IsFullScreen;
	i32 WindowWidth;
	i32 WindowHeight;

	i64 PerformanceFrequence;

	char ExeFileName[PATH_MAX];
	char *OnePastLastEXEFileNameSlash;
};
