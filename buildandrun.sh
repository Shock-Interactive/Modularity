#!/bin/bash
set -e  # Exit on any error

./build.sh || { echo "Build failed"; exit 1; }
cd build || { echo "Cannot cd to build/"; exit 1; }
./main || { echo "main failed"; exit 1; }
cd .. || echo "Warning: failed to return to parent dir"