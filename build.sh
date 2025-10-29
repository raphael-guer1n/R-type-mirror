#!/bin/bash
set -e

echo "Cleaning build directory..."
rm -rf build
rm -rf vcpkg_installed

echo "Configuring project with CMake..."
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_CXX_COMPILER=/usr/bin/c++

echo "Building project..."
cmake --build build --parallel

echo "Build complete!"
