#!/bin/bash
echo "Build started"  
g++ ./Linux/main.cpp -std=c++17 -g -lSDL2 -lSDL2main -o ./Build/Linux/main
echo "Build finished"  