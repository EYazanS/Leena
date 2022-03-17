#!/bin/bash
echo "Build started"  
g++ ./src/linux.cpp -std=c++17 -g -lSDL2 -lSDL2main -o ./build/linux.out
echo "Build finished"  
