#!/bin/bash

if [ -d "build" ]; then
    echo "found existing!! Removing..."
    rm -rf build/
    mkdir build
    cd build
    cmake ..
    cmake --build . -- -j$(nproc)
    cp -r ../Shaders .
    echo "Build Done!"
else
    mkdir build
    cd build
    cmake ..
    cmake --build . -- -j$(nproc)
    cp -r ../Shaders .
    echo "Build Done!"
fi