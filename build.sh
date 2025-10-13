#!/bin/bash
set -e

echo "Cleaning build directory..."
rm -rf build

echo "Configuring project with CMake..."
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

echo "Building project..."
cmake --build build

echo "✅ Build complete!"

if [ -f "./tests/unit_tests" ]; then
    echo ""
    echo "=============================="
    echo " Running Unit Tests 🎯"
    echo "=============================="
    ./tests/unit_tests
    echo "=============================="
    echo " Unit Tests Finished ✅"
    echo "=============================="
else
    echo "⚠️  No tests found in ./tests/"
fi