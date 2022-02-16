#!/bin/sh

# Сборка примеров
echo "Building examples..."
mkdir -p build/Debug
cd build/Debug || exit 1
cmake ../..
cmake --build .
