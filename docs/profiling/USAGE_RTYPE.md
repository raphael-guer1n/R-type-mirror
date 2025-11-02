# R-Type Profiling Usage Guide

## Overview

The profiling system has been fully integrated into your R-Type project for both client and server. This allows you to monitor performance metrics, debug bottlenecks, and optimize your game.

## Client Usage

### Running the Client with Profiling

When you run the client, profiling is automatically enabled:

```bash
./r-type_client [SERVER_IP] [PORT]
```

### Keyboard Controls

- **F3**: Toggle the profiler overlay on/off (default: ON)
- **Ctrl+B**: Toggle hitbox overlay (existing feature)

### What You'll See

The profiler overlay displays in the top-left corner:
- **FPS**: Current frames per second
- **Frame Time**: Current frame time and average (in milliseconds)
- **Memory**: Physical memory usage in MB
- **Network**: Latency and packet statistics
- **Entities**: Number of active game entities
- **Position**: Your player's world coordinates
- **Network Send/Receive**: Timing for network operations

### Example Output

```
FPS: 60.0
Frame: 16.67ms (avg: 16.70ms)
Memory: 45.2MB
Latency: 12.5ms (avg: 13.2ms)
Packets: 120/118 (dropped: 0)
Entities: 25
Pos: (234.5, 540.0)
Network Send: 0.15ms
Network Receive: 0.32ms
Game Systems: 2.45ms
```

## Server Usage

### Running the Server with Profiling

```bash
./r-type_server [PORT]
```

The server automatically logs profiling stats to the console every 5 seconds.

### Server Console Output

You'll see periodic logs like:

```
[Profiling] FPS: 60.0 | Frame: 16.67ms | Entities: 42 | Memory: 28.5MB
```

This includes:
- Server tick rate (target: 60 FPS)
- Frame processing time
- Number of active entities
- Memory consumption

### Profiled Sections

The server tracks timing for:
- **Network Input**: Processing player inputs
- **Game Tick**: Overall game logic
- **Game Handler**: Enemy spawning and game events
- **Physics Systems**: Movement and collision
- **Broadcast Snapshot**: Sending state to clients

## Advanced Usage

### Disabling Profiling

If you want to disable profiling (e.g., for release builds), you can:

1. **At compile time**: Edit `CMakeLists.txt`
   ```cmake
   set(ENGINE_PROFILING OFF)
   ```

2. **At runtime** (client only):
   ```cpp
   // In code
   auto& profiler = Engine::Profiling::Profiler::getInstance();
   profiler.setEnabled(false);
   ```

### Custom Profiling Scopes

Add profiling to your own code:

```cpp
#ifdef ENGINE_HAS_PROFILING
#include "engine/profiling/Profiler.hpp"

void myFunction() {
    PROFILE_FUNCTION(); // Profiles entire function
    // Your code...
}

void myOtherFunction() {
    // Profile specific section
    {
        PROFILE_SCOPE("Custom Section");
        // Code to profile...
    }
}
#endif
```

### Analyzing Performance

#### Finding Bottlenecks

1. **Watch Frame Time**: If it exceeds 16.67ms, you're below 60 FPS
2. **Check Scope Times**: See which systems take the most time
3. **Monitor Entity Count**: High entity counts can impact performance
4. **Track Memory**: Look for memory leaks or excessive allocation

#### Common Issues

- **High "Network Receive" time**: May indicate network congestion or packet loss
- **High "Game Systems" time**: Physics or collision detection bottleneck
- **High "Broadcast Snapshot" time**: Too many entities being synchronized
- **Increasing Memory**: Possible memory leak

## Customizing the Overlay

You can customize what the profiler displays by modifying `Rtype.cpp`:

```cpp
Engine::Profiling::ProfilerDisplayConfig config;
config.showFPS = true;           // Show FPS
config.showFrameTime = true;     // Show frame timing
config.showMemory = true;        // Show memory usage
config.showCPU = false;          // Hide CPU (can be expensive to calculate)
config.showNetwork = true;       // Show network stats
config.showWorld = true;         // Show world/entity info
config.showScopes = true;        // Show custom scope timings
config.posX = 10;                // X position
config.posY = 10;                // Y position
config.fontSize = 14;            // Font size
```

## Performance Tips

1. **Update heavy metrics sparingly**: CPU and memory metrics are updated every 30 frames
2. **Toggle overlay when not needed**: Press F3 to hide during normal gameplay
3. **Use scopes strategically**: Profile only the sections you're optimizing
4. **Watch for patterns**: Look for frame time spikes that correlate with game events

## Benchmarking

To benchmark specific features:

1. Enable profiling
2. Run a test scenario (e.g., spawn many enemies)
3. Note the FPS and frame times
4. Use scope profiling to identify bottlenecks
5. Optimize and re-test

## Example: Profiling a New Feature

```cpp
void MyNewGameFeature::update() {
#ifdef ENGINE_HAS_PROFILING
    PROFILE_SCOPE("MyNewFeature");
#endif
    
    // Your feature code...
    
#ifdef ENGINE_HAS_PROFILING
    auto& profiler = Engine::Profiling::Profiler::getInstance();
    double time = profiler.getScopeTime("MyNewFeature");
    if (time > 5.0) {
        std::cout << "Warning: MyNewFeature taking " << time << "ms\n";
    }
#endif
}
```

## Troubleshooting

**Overlay not showing?**
- Press F3 to toggle
- Check that `Assets/fonts/arial.ttf` exists
- Verify ENGINE_HAS_PROFILING is defined

**Server not logging stats?**
- Wait 5 seconds for first log
- Check ENGINE_HAS_PROFILING is defined

**High CPU usage from profiling?**
- Disable CPU metrics (`config.showCPU = false`)
- Reduce profiling frequency
- Disable profiling for release builds

## Compilation

The profiling system compiles automatically if enabled in CMakeLists.txt:

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

The profiling module is included in the default `ENGINE_MODULES` configuration.
