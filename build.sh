#!/bin/bash

if [ -d "build" ]; then
    echo ================================
    echo   Modularity - VS 2026 Build
    echo ================================
    git submodule update --init --recursive
    echo "found existing!! Removing..."
    rm -rf build/
    mkdir build
    cd build
    cmake ..
    cmake --build . -- -j$(nproc)
    cp -r ../Resources .
    echo "Build Done!"
else
    mkdir build
    cd build
    cmake ..
    cmake --build . -- -j$(nproc)
    cp -r ../Resources .
    echo "Build Done!"
fi