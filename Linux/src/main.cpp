#include <iostream>
#include <SDL2/SDL.h>

int main(int argc, char **argv)
{
    std::cout << "Debugging\n";
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Quit();

    return 0;
}