#include "main.h"

int main(int argc, char **argv)
{
    LinuxProgramState programState = {};

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

    ThreadContext thread = {};

    GameCode game = LinuxLoadGameCode();
    GameMemory gameMemory = InitGameMemory();

    GameInput Input[2] = {};

    GameInput *previousInput = &Input[0];
    GameInput *currentInput = &Input[1];

    GameAudioBuffer gameaudioBuffer = {};

    programState.IsRunning = true;

    real64 timeTookToRenderLastFrame = 0.0f;

    while (programState.IsRunning)
    {
        if (0)
        {
            LinuxUnloadGameCode(&game);
            game = LinuxLoadGameCode();
        }

        // Process the keyboard input.
        KeyboardInput *oldKeyboardInput = &previousInput->Keyboard;
        KeyboardInput *newKeyboardInput = &currentInput->Keyboard;

        *newKeyboardInput = {};
        newKeyboardInput->IsConnected = true;

        // Keep the state of the down button from the past frame.
        for (int buttonIndex = 0; buttonIndex < ArrayCount(newKeyboardInput->Buttons); buttonIndex++)
            newKeyboardInput->Buttons[buttonIndex].EndedDown = oldKeyboardInput->Buttons[buttonIndex].EndedDown;

        SDL_Event e;

        while (SDL_PollEvent(&e) > 0)
        {
            switch (e.type)
            {
            case SDL_QUIT:
                programState.IsRunning = false;
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                ProccessKeyboardKeys(&programState, e, newKeyboardInput);
            }
            break;
            }

            SDL_UpdateWindowSurface(window);
        }

        // Process the mouse input
        LinuxGetMouseState(&currentInput->Mouse);

        // Proccess screen buffer
        GameScreenBuffer screenBuffer = {};

        game.Update(&thread, &gameMemory, &screenBuffer, &gameaudioBuffer, currentInput);
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

    gameMemory.PermenantStorageSize = Megabytes(64);
    gameMemory.TransiateStorageSize = Gigabytes(1);

    uint64 totalSize = gameMemory.PermenantStorageSize + gameMemory.TransiateStorageSize;

    gameMemory.PermenantStorage = malloc(totalSize);
    gameMemory.TransiateStorage = (uint8 *)gameMemory.PermenantStorage + gameMemory.PermenantStorageSize;

    // gameMemory.FreeFile = DebugPlatformFreeFileMemory;
    // gameMemory.ReadFile = DebugPlatformReadEntireFile;
    // gameMemory.WriteFile = DebugPlatformWriteEntireFile;

    return gameMemory;
}

void LinuxUnloadGameCode(GameCode *gameCode)
{
    if (gameCode->LibraryHandle)
        dlclose(gameCode->LibraryHandle);

    gameCode->IsValid = false;
    gameCode->Update = GameUpdateStub;
}

void ProccessKeyboardKeys(LinuxProgramState *state, SDL_Event &event, KeyboardInput *controller)
{
    uint32 vkCode = static_cast<uint32>(event.key.keysym.sym);

    bool isPressed = event.type == SDL_KEYDOWN;

    switch (vkCode)
    {
    case SDLK_w:
    {
        LinuxProccessKeyboardMessage(controller->W, isPressed, event.key.repeat);
    }
    break;

    case SDLK_a:
    {
        LinuxProccessKeyboardMessage(controller->A, isPressed, event.key.repeat);
    }
    break;

    case SDLK_d:
    {
        LinuxProccessKeyboardMessage(controller->D, isPressed, event.key.repeat);
    }
    break;

    case SDLK_s:
    {
        LinuxProccessKeyboardMessage(controller->S, isPressed, event.key.repeat);
    }
    break;

    case SDLK_q:
    {
        LinuxProccessKeyboardMessage(controller->Q, isPressed, event.key.repeat);
    }
    break;

    case SDLK_e:
    {
        LinuxProccessKeyboardMessage(controller->E, isPressed, event.key.repeat);
    }
    break;

    default:
    {
    }
    break;
    }
}

void LinuxProccessKeyboardMessage(GameButtonState &state, bool32 isPressed, int repeat)
{
    state.EndedDown = isPressed;
    state.HalfTransitionCount = repeat;
}

void LinuxGetMouseState(MouseInput *mouse)
{
    int mouseState = SDL_GetMouseState((int *)(&mouse->X), (int *)(&mouse->Y));
    mouse->LeftButton.EndedDown = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
    mouse->RightButton.EndedDown = mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT);
}