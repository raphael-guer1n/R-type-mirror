#!/bin/bash
set -e

# Pick a generator
GENERATOR=""
if command -v ninja >/dev/null 2>&1; then
  GENERATOR="-G Ninja"
elif command -v make >/dev/null 2>&1; then
  GENERATOR="-G Unix Makefiles"
else
  echo "No build tool found. Please install ninja or make, and a C++ compiler (e.g., gcc-c++)." >&2
  exit 1
fi

echo "Cleaning build directory..."
rm -rf build
# Also clean cached install to re-resolve manifest features
rm -rf vcpkg_installed

echo "Configuring project with CMake..."
cmake -B build -S . ${GENERATOR} -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

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