# Engine Profiling System

## Overview

The profiling subsystem provides comprehensive in-game metrics and performance monitoring for debugging, benchmarking, and optimization. It includes FPS tracking, CPU/Memory usage, network statistics, world metrics, and custom scope profiling.

## Features

- **Frame Metrics**: FPS, frame time (current, min, max, average)
- **System Metrics**: CPU usage, memory consumption (physical, virtual, peak)
- **Network Metrics**: Latency tracking, packet statistics, bandwidth monitoring
- **World Metrics**: Entity count, world position tracking, active systems
- **Scope Profiling**: Automatic and manual timing of code sections
- **Visual Overlay**: Optional on-screen display of metrics (requires renderer)

## Basic Usage

### 1. Frame Timing

```cpp
#include "engine/Engine.hpp"

auto& profiler = Engine::Profiling::Profiler::getInstance();

// In your game loop
while (running) {
    profiler.beginFrame();
    
    // Your game logic here...
    
    profiler.endFrame();
    
    // Access metrics
    const auto& metrics = profiler.getFrameMetrics();
    std::cout << "FPS: " << metrics.fps << std::endl;
}
```

### 2. Scope Profiling

```cpp
// Automatic scope timing with RAII
{
    PROFILE_SCOPE("Physics Update");
    // Physics code is automatically timed
}

// Function profiling
void myFunction() {
    PROFILE_FUNCTION(); // Profiles entire function
    // ...
}

// Manual scope timing
profiler.beginScope("Custom Section");
// ... code to profile ...
profiler.endScope("Custom Section");
double time = profiler.getScopeTime("Custom Section");
```

### 3. System Metrics

```cpp
// Update periodically (not every frame for performance)
profiler.updateMemoryMetrics();
profiler.updateCPUMetrics();

const auto& mem = profiler.getMemoryMetrics();
const auto& cpu = profiler.getCPUMetrics();

std::cout << "Memory: " << (mem.physicalMemoryUsed / 1024.0 / 1024.0) << " MB\n";
std::cout << "CPU: " << cpu.cpuUsagePercent << "%\n";
```

### 4. Network Profiling

```cpp
// Record network activity
profiler.recordPacketSent(bytesCount);
profiler.recordPacketReceived(bytesCount);
profiler.recordLatency(latencyMs);

const auto& net = profiler.getNetworkMetrics();
std::cout << "Avg Latency: " << net.avgLatency << "ms\n";
```

### 5. Visual Overlay (Requires Renderer)

```cpp
#ifdef ENGINE_HAS_RENDERER
Engine::Profiling::ProfilerOverlay overlay;
overlay.initialize(renderer, "Assets/fonts/arial.ttf");

// Configure what to display
Engine::Profiling::ProfilerDisplayConfig config;
config.showFPS = true;
config.showMemory = true;
config.showNetwork = true;
config.posX = 10;
config.posY = 10;
overlay.setConfig(config);

// In render loop
overlay.render();

// Toggle visibility with F3 or similar
if (keyPressed(KEY_F3)) {
    overlay.toggleVisibility();
}
#endif
```

## Data Structures

### FrameMetrics
- `fps`: Current frames per second
- `frameTime`: Current frame time in milliseconds
- `avgFrameTime`: Average frame time over last 120 frames
- `minFrameTime`: Minimum frame time
- `maxFrameTime`: Maximum frame time
- `frameCount`: Total frames rendered

### MemoryMetrics
- `physicalMemoryUsed`: Physical RAM usage in bytes
- `virtualMemoryUsed`: Virtual memory usage in bytes
- `peakMemoryUsed`: Peak memory usage in bytes

### CPUMetrics
- `cpuUsagePercent`: CPU usage percentage
- `numCores`: Number of CPU cores

### NetworkMetrics
- `latency`: Current latency in milliseconds
- `avgLatency`: Average latency
- `maxLatency`: Maximum latency
- `packetsDropped`: Number of dropped packets
- `packetsSent`: Total packets sent
- `packetsReceived`: Total packets received
- `bytesSent`: Total bytes sent
- `bytesReceived`: Total bytes received

### WorldMetrics
- `positionX/Y/Z`: World position coordinates
- `entityCount`: Number of active entities
- `activeSystemCount`: Number of active systems

## Advanced Features

### Print Summary

```cpp
std::string summary = profiler.getSummary();
std::cout << summary << std::endl;
```

### Reset Profiler

```cpp
profiler.reset(); // Clear all metrics
```

### Enable/Disable

```cpp
profiler.setEnabled(false); // Disable for release builds
profiler.setEnabled(true);  // Re-enable
```

## Platform Support

The profiling system supports:
- **Linux**: Uses `/proc` filesystem for metrics
- **Windows**: Uses Windows API (GetProcessMemoryInfo, GetProcessTimes)
- **macOS**: Uses Mach kernel APIs

## Performance Considerations

- Frame metrics have minimal overhead (< 0.1ms)
- System metrics (CPU/Memory) should be updated periodically, not every frame
- Profiling can be completely disabled with `setEnabled(false)`
- Visual overlay has rendering cost; toggle off when not needed

## Integration Example

```cpp
// Complete game loop with profiling
auto& profiler = Engine::Profiling::Profiler::getInstance();

#ifdef ENGINE_HAS_RENDERER
Engine::Profiling::ProfilerOverlay overlay;
overlay.initialize(renderer, "Assets/fonts/arial.ttf");
#endif

while (running) {
    profiler.beginFrame();
    
    {
        PROFILE_SCOPE("Input");
        handleInput();
    }
    
    {
        PROFILE_SCOPE("Physics");
        updatePhysics();
    }
    
    {
        PROFILE_SCOPE("Render");
        render();
        #ifdef ENGINE_HAS_RENDERER
        overlay.render();
        #endif
    }
    
    // Update system metrics every 30 frames
    if (profiler.getFrameMetrics().frameCount % 30 == 0) {
        profiler.updateMemoryMetrics();
        profiler.updateCPUMetrics();
    }
    
    profiler.endFrame();
}
```

## Compilation

The profiling module is enabled by default. To disable:

```cmake
set(ENGINE_PROFILING OFF)
```

Or use preprocessor directives:

```cpp
#ifdef ENGINE_HAS_PROFILING
// Profiling code
#endif
```
