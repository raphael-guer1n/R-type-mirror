
@echo "Cleaning build directory..."
@rmdir /s /q build
@rmdir /s /q Release
@rmdir /s /q Debug
@mkdir build

@echo "Configuring the project with CMake..."
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -T v143 -DCMAKE_TOOLCHAIN_FILE="C:\Users\Alexi\OneDrive\Bureau\R-type-mirror\vcpkg\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows

@echo "Building project..."
cmake --build build --config Release

@echo "Creating junctions for assets/configs in build output folders..."
@cd Release
@mklink /J Assets "..\Assets"
@mklink /J configs "..\configs"
@cd ..

@echo "Build complete!"