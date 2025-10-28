#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>

namespace Engine {
namespace Profiling {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<double, std::milli>;

struct FrameMetrics {
    double fps = 0.0;
    double frameTime = 0.0;
    double minFrameTime = 0.0;
    double maxFrameTime = 0.0;
    double avgFrameTime = 0.0;
    uint64_t frameCount = 0;
    double displayFps = 0.0;
    double displayFrameTime = 0.0;
};

struct MemoryMetrics {
    size_t virtualMemoryUsed = 0;
    size_t physicalMemoryUsed = 0;
    size_t peakMemoryUsed = 0;
};

struct CPUMetrics {
    double cpuUsagePercent = 0.0;
    uint32_t numCores = 0;
};

struct NetworkMetrics {
    double latency = 0.0;
    double avgLatency = 0.0;
    double maxLatency = 0.0;
    uint64_t packetsDropped = 0;
    uint64_t packetsSent = 0;
    uint64_t packetsReceived = 0;
    uint64_t bytesSent = 0;
    uint64_t bytesReceived = 0;
};

struct WorldMetrics {
    float positionX = 0.0f;
    float positionY = 0.0f;
    float positionZ = 0.0f;
    uint32_t entityCount = 0;
    uint32_t activeSystemCount = 0;
};

struct ScopeTimer {
    std::string name;
    TimePoint startTime;
    bool active;

    ScopeTimer(const std::string& name);
    ~ScopeTimer();
    
    void stop();
};

class Profiler {
public:
    static Profiler& getInstance();

    void beginFrame();
    void endFrame();
    const FrameMetrics& getFrameMetrics() const { return _frameMetrics; }

    void beginScope(const std::string& name);
    void endScope(const std::string& name);
    double getScopeTime(const std::string& name) const;
    const std::unordered_map<std::string, double>& getAllScopeTimes() const { return _scopeTimes; }

    void updateMemoryMetrics();
    void updateCPUMetrics();
    const MemoryMetrics& getMemoryMetrics() const { return _memoryMetrics; }
    const CPUMetrics& getCPUMetrics() const { return _cpuMetrics; }

    void recordPacketSent(size_t bytes);
    void recordPacketReceived(size_t bytes);
    void recordPacketDropped();
    void recordLatency(double latency);
    const NetworkMetrics& getNetworkMetrics() const { return _networkMetrics; }

    void setWorldPosition(float x, float y, float z = 0.0f);
    void setEntityCount(uint32_t count);
    void setActiveSystemCount(uint32_t count);
    const WorldMetrics& getWorldMetrics() const { return _worldMetrics; }

    void reset();
    void setEnabled(bool enabled) { _enabled = enabled; }
    bool isEnabled() const { return _enabled; }

    std::string getSummary() const;

private:
    Profiler();
    ~Profiler() = default;
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    bool _enabled = true;
    
    FrameMetrics _frameMetrics;
    TimePoint _frameStartTime;
    std::vector<double> _frameTimeHistory;
    static constexpr size_t MAX_FRAME_HISTORY = 120;
    
    TimePoint _lastDisplayUpdate;
    uint64_t _framesInLastSecond = 0;
    double _frameTimeAccumulator = 0.0;

    std::unordered_map<std::string, TimePoint> _scopeStarts;
    std::unordered_map<std::string, double> _scopeTimes;

    MemoryMetrics _memoryMetrics;
    CPUMetrics _cpuMetrics;
    TimePoint _lastCPUCheckTime;

    NetworkMetrics _networkMetrics;
    std::vector<double> _latencyHistory;
    static constexpr size_t MAX_LATENCY_HISTORY = 100;

    WorldMetrics _worldMetrics;
};

#define PROFILE_SCOPE(name) Engine::Profiling::ScopeTimer _profiler_timer_##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)

}
}
