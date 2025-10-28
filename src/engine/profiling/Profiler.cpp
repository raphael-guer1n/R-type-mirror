#include "engine/profiling/Profiler.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <thread>

namespace Engine {
namespace Profiling {

ScopeTimer::ScopeTimer(const std::string& name) 
    : name(name), startTime(Clock::now()), active(true) {
    Profiler::getInstance().beginScope(name);
}

ScopeTimer::~ScopeTimer() {
    if (active) {
        stop();
    }
}

void ScopeTimer::stop() {
    if (active) {
        Profiler::getInstance().endScope(name);
        active = false;
    }
}

Profiler::Profiler() {
    _frameStartTime = Clock::now();
    _lastCPUCheckTime = Clock::now();
    _lastDisplayUpdate = Clock::now();
    _frameTimeHistory.reserve(MAX_FRAME_HISTORY);
    _latencyHistory.reserve(MAX_LATENCY_HISTORY);
    _cpuMetrics.numCores = std::thread::hardware_concurrency();
}

Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::beginFrame() {
    if (!_enabled) return;
    _frameStartTime = Clock::now();
}

void Profiler::endFrame() {
    if (!_enabled) return;
    
    auto frameEndTime = Clock::now();
    Duration frameDuration = frameEndTime - _frameStartTime;
    double frameTime = frameDuration.count();
    
    _frameMetrics.frameTime = frameTime;
    _frameMetrics.frameCount++;
    _framesInLastSecond++;
    _frameTimeAccumulator += frameTime;
    Duration timeSinceLastUpdate = frameEndTime - _lastDisplayUpdate;
    if (timeSinceLastUpdate.count() >= 1000.0) {
        if (_framesInLastSecond > 0) {
            _frameMetrics.displayFps = _framesInLastSecond / (timeSinceLastUpdate.count() / 1000.0);
            _frameMetrics.displayFrameTime = _frameTimeAccumulator / _framesInLastSecond;
        }
        _framesInLastSecond = 0;
        _frameTimeAccumulator = 0.0;
        _lastDisplayUpdate = frameEndTime;
    }
    if (frameTime > 0.0)
        _frameMetrics.fps = 1000.0 / frameTime;
    _frameTimeHistory.push_back(frameTime);
    if (_frameTimeHistory.size() > MAX_FRAME_HISTORY) {
        _frameTimeHistory.erase(_frameTimeHistory.begin());
    }
    if (!_frameTimeHistory.empty()) {
        _frameMetrics.minFrameTime = *std::min_element(_frameTimeHistory.begin(), _frameTimeHistory.end());
        _frameMetrics.maxFrameTime = *std::max_element(_frameTimeHistory.begin(), _frameTimeHistory.end());
        _frameMetrics.avgFrameTime = std::accumulate(_frameTimeHistory.begin(), _frameTimeHistory.end(), 0.0) / _frameTimeHistory.size();
    }
}

void Profiler::beginScope(const std::string& name) {
    if (!_enabled) return;
    _scopeStarts[name] = Clock::now();
}

void Profiler::endScope(const std::string& name) {
    if (!_enabled) return;
    
    auto it = _scopeStarts.find(name);
    if (it != _scopeStarts.end()) {
        auto endTime = Clock::now();
        Duration duration = endTime - it->second;
        _scopeTimes[name] = duration.count();
        _scopeStarts.erase(it);
    }
}

double Profiler::getScopeTime(const std::string& name) const {
    auto it = _scopeTimes.find(name);
    return (it != _scopeTimes.end()) ? it->second : 0.0;
}

void Profiler::updateMemoryMetrics() {
    if (!_enabled) return;
    
    std::ifstream statm("/proc/self/statm");
    long vmsize, rss;
    if (statm >> vmsize >> rss) {
        // Assume 4KB page size (standard on most Linux systems)
        constexpr long pageSize = 4096;
        _memoryMetrics.virtualMemoryUsed = vmsize * pageSize;
        _memoryMetrics.physicalMemoryUsed = rss * pageSize;
        _memoryMetrics.peakMemoryUsed = std::max(_memoryMetrics.peakMemoryUsed, _memoryMetrics.physicalMemoryUsed);
    }
}

void Profiler::updateCPUMetrics() {
    if (!_enabled) return;
    
    static long lastTotalTime = 0;
    static long lastProcTime = 0;
    
    std::ifstream stat("/proc/stat");
    std::string cpu;
    long user, nice, system, idle;
    stat >> cpu >> user >> nice >> system >> idle;
    long totalTime = user + nice + system + idle;
    
    std::ifstream procStat("/proc/self/stat");
    std::string skip;
    long utime, stime;
    for (int i = 0; i < 13; ++i) procStat >> skip;
    procStat >> utime >> stime;
    long procTime = utime + stime;
    
    if (lastTotalTime > 0) {
        long totalDiff = totalTime - lastTotalTime;
        long procDiff = procTime - lastProcTime;
        if (totalDiff > 0) {
            _cpuMetrics.cpuUsagePercent = (100.0 * procDiff) / totalDiff;
        }
    }
    
    lastTotalTime = totalTime;
    lastProcTime = procTime;
}

void Profiler::recordPacketSent(size_t bytes) {
    if (!_enabled) return;
    _networkMetrics.packetsSent++;
    _networkMetrics.bytesSent += bytes;
}

void Profiler::recordPacketReceived(size_t bytes) {
    if (!_enabled) return;
    _networkMetrics.packetsReceived++;
    _networkMetrics.bytesReceived += bytes;
}

void Profiler::recordPacketDropped() {
    if (!_enabled) return;
    _networkMetrics.packetsDropped++;
}

void Profiler::recordLatency(double latency) {
    if (!_enabled) return;
    
    _networkMetrics.latency = latency;
    
    _latencyHistory.push_back(latency);
    if (_latencyHistory.size() > MAX_LATENCY_HISTORY) {
        _latencyHistory.erase(_latencyHistory.begin());
    }
    
    if (!_latencyHistory.empty()) {
        _networkMetrics.avgLatency = std::accumulate(_latencyHistory.begin(), _latencyHistory.end(), 0.0) / _latencyHistory.size();
        _networkMetrics.maxLatency = *std::max_element(_latencyHistory.begin(), _latencyHistory.end());
    }
}

void Profiler::setWorldPosition(float x, float y, float z) {
    if (!_enabled) return;
    _worldMetrics.positionX = x;
    _worldMetrics.positionY = y;
    _worldMetrics.positionZ = z;
}

void Profiler::setEntityCount(uint32_t count) {
    if (!_enabled) return;
    _worldMetrics.entityCount = count;
}

void Profiler::setActiveSystemCount(uint32_t count) {
    if (!_enabled) return;
    _worldMetrics.activeSystemCount = count;
}

void Profiler::reset() {
    _frameMetrics = {};
    _memoryMetrics = {};
    _networkMetrics = {};
    _worldMetrics = {};
    _scopeTimes.clear();
    _scopeStarts.clear();
    _frameTimeHistory.clear();
    _latencyHistory.clear();
}

std::string Profiler::getSummary() const {
    std::stringstream ss;
    
    ss << "=== Profiler Summary ===\n";
    ss << "Frame Metrics:\n";
    ss << "  FPS: " << std::fixed << std::setprecision(2) << _frameMetrics.fps << "\n";
    ss << "  Frame Time: " << _frameMetrics.frameTime << " ms\n";
    ss << "  Avg Frame Time: " << _frameMetrics.avgFrameTime << " ms\n";
    ss << "  Min/Max Frame Time: " << _frameMetrics.minFrameTime << " / " << _frameMetrics.maxFrameTime << " ms\n";
    ss << "  Total Frames: " << _frameMetrics.frameCount << "\n\n";
    
    ss << "Memory Metrics:\n";
    ss << "  Physical Memory: " << (_memoryMetrics.physicalMemoryUsed / 1024.0 / 1024.0) << " MB\n";
    ss << "  Virtual Memory: " << (_memoryMetrics.virtualMemoryUsed / 1024.0 / 1024.0) << " MB\n";
    ss << "  Peak Memory: " << (_memoryMetrics.peakMemoryUsed / 1024.0 / 1024.0) << " MB\n\n";
    
    ss << "CPU Metrics:\n";
    ss << "  CPU Usage: " << _cpuMetrics.cpuUsagePercent << "%\n";
    ss << "  CPU Cores: " << _cpuMetrics.numCores << "\n\n";
    
    ss << "Network Metrics:\n";
    ss << "  Latency: " << _networkMetrics.latency << " ms\n";
    ss << "  Avg Latency: " << _networkMetrics.avgLatency << " ms\n";
    ss << "  Max Latency: " << _networkMetrics.maxLatency << " ms\n";
    ss << "  Packets Sent/Received: " << _networkMetrics.packetsSent << " / " << _networkMetrics.packetsReceived << "\n";
    ss << "  Packets Dropped: " << _networkMetrics.packetsDropped << "\n";
    ss << "  Bytes Sent/Received: " << _networkMetrics.bytesSent << " / " << _networkMetrics.bytesReceived << "\n\n";
    
    ss << "World Metrics:\n";
    ss << "  Position: (" << _worldMetrics.positionX << ", " << _worldMetrics.positionY << ", " << _worldMetrics.positionZ << ")\n";
    ss << "  Entity Count: " << _worldMetrics.entityCount << "\n";
    ss << "  Active Systems: " << _worldMetrics.activeSystemCount << "\n\n";
    
    if (!_scopeTimes.empty()) {
        ss << "Scope Times:\n";
        for (const auto& [name, time] : _scopeTimes) {
            ss << "  " << name << ": " << time << " ms\n";
        }
    }
    
    return ss.str();
}

} // namespace Profiling
} // namespace Engine
