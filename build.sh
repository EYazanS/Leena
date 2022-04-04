#!/bin/bash
echo "Build started"  
mkdir -p ./build
g++ ./src/linux.cpp -std=gnu++17 -Wall -fno-rtti -fno-exceptions -g -DLeena_Internal=1 -DLeena_SDL=1 -lSDL2 -lSDL2main -o ./build/linux.out
echo "Build finished"  
