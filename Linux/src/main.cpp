#include "main.h"

int main(int argc, char **argv)
{
    LinuxProgramState programState = {true};

    SDL_Window *window = CreateWindow();

    if (!window)
    {
        std::cout << "Failed to create window\n";
        return -1;
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);

    if (!window_surface)
    {
        std::cout << "Failed to get the surface from the window\n";
        return -1;
    }

    GameCode gameCode = LinuxLoadGameCode();
    GameMemory gameMemory = InitGameMemory();

    while (programState.IsRunning)
    {
        SDL_Event e;

        while (SDL_PollEvent(&e) > 0)
        {
            switch (e.type)
            {
            case SDL_QUIT:
                programState.IsRunning = false;
                break;
            }

            SDL_UpdateWindowSurface(window);
        }
    }

    SDL_Quit();

    return 0;
}

SDL_Window *CreateWindow()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Failed to initialize the SDL2 library\n";
        return 0;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Leena Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        680, 480,
        0);

    return window;
}

GameCode LinuxLoadGameCode()
{
    char *error;

    std::ifstream src("./Build/Linux/Leena.dll", std::ios::binary);
    std::ofstream dst("./Build/Linux/Tmp.dll", std::ios::binary);

    dst << src.rdbuf();

    void *gameCodeHandle = dlopen("./Build/Linux/Tmp.dll", RTLD_LAZY);

    std::cout << "error: " << dlerror() << std::endl;

    GameCode result = {gameCodeHandle, GameUpdateStub};

    if (gameCodeHandle)
    {
        result.Update = (GAMEUPDATE *)dlsym(gameCodeHandle, "GameUpdate");

        if ((error = dlerror()) == NULL)
            result.IsValid = true;
        else
            result.IsValid = false;

        std::cout << "error: " << error << std::endl;
    }

    return result;
}

GameMemory InitGameMemory()
{
    GameMemory gameMemory = {};

#if Leena_Internal
    void *baseAddress = (void *)Terabytes(2);
#else
    void *baseAddress = NULL;
#endif

    gameMemory.PermenantStorageSize = Megabytes(64);
    gameMemory.TransiateStorageSize = Gigabytes(1);

    uint64 totalSize = gameMemory.PermenantStorageSize + gameMemory.TransiateStorageSize;

    gameMemory.PermenantStorage = mmap(baseAddress, totalSize, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, 0, 0);
    gameMemory.TransiateStorage = (uint8 *)gameMemory.PermenantStorage + gameMemory.PermenantStorageSize;

    // gameMemory.FreeFile = DebugPlatformFreeFileMemory;
    // gameMemory.ReadFile = DebugPlatformReadEntireFile;
    // gameMemory.WriteFile = DebugPlatformWriteEntireFile;

    return gameMemory;
}