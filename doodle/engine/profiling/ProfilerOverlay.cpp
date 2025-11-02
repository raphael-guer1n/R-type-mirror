#include "engine/profiling/ProfilerOverlay.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <sstream>
#include <iomanip>

namespace Engine {
namespace Profiling {

ProfilerOverlay::ProfilerOverlay() = default;

ProfilerOverlay::~ProfilerOverlay() {
    if (_font) {
        TTF_CloseFont(static_cast<TTF_Font*>(_font));
        _font = nullptr;
    }
}

bool ProfilerOverlay::initialize(SDL_Renderer* renderer, const std::string& fontPath) {
    if (!renderer)
        return false;
    _renderer = renderer;
    if (!TTF_WasInit() && TTF_Init() == -1)
        return false;
    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), _config.fontSize);
    if (!font)
        return false;
    _font = static_cast<TTF_FontPtr>(font);
    _initialized = true;
    return true;
}

void ProfilerOverlay::render() {
    if (!_initialized || !_visible || !_renderer)
        return;
    auto metrics = formatMetrics();
    if (metrics.empty())
        return;
    int maxWidth = 0;
    int lineHeight = _config.fontSize + 4;
    int totalHeight = metrics.size() * lineHeight + 10;
    
    for (const auto& line : metrics) {
        int textWidth = line.length() * (_config.fontSize / 2 + 2);
        maxWidth = std::max(maxWidth, textWidth);
    }
    renderBackground(_config.posX - 5, _config.posY - 5, maxWidth + 10, totalHeight);
    int y = _config.posY;

    for (const auto& line : metrics) {
        renderText(line, _config.posX, y);
        y += lineHeight;
    }
}

void ProfilerOverlay::renderText(const std::string& text, int x, int y) {
    if (!_font || text.empty())
        return;
    
    TTF_Font* font = static_cast<TTF_Font*>(_font);
    SDL_Color color = {_config.textColor.r, _config.textColor.g, _config.textColor.b, _config.textColor.a};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface)
        return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
    if (texture) {
        SDL_Rect dstRect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(_renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void ProfilerOverlay::renderBackground(int x, int y, int width, int height) {
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer, 
                          _config.backgroundColor.r, 
                          _config.backgroundColor.g, 
                          _config.backgroundColor.b, 
                          _config.backgroundColor.a);
    
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderFillRect(_renderer, &rect);
}

std::vector<std::string> ProfilerOverlay::formatMetrics() {
    std::vector<std::string> lines;
    auto& profiler = Profiler::getInstance();
    std::stringstream ss;
    
    if (_config.showFPS) {
        const auto& frame = profiler.getFrameMetrics();
        ss << "FPS: " << std::fixed << std::setprecision(1) << frame.displayFps;
        lines.push_back(ss.str());
        ss.str("");
    }
    
    if (_config.showFrameTime) {
        const auto& frame = profiler.getFrameMetrics();
        ss << "Frame: " << std::fixed << std::setprecision(2) << frame.displayFrameTime << "ms"
           << " (avg: " << frame.avgFrameTime << "ms)";
        lines.push_back(ss.str());
        ss.str("");
    }
    
    if (_config.showMemory) {
        const auto& mem = profiler.getMemoryMetrics();
        ss << "Memory: " << std::fixed << std::setprecision(1) 
           << (mem.physicalMemoryUsed / 1024.0 / 1024.0) << "MB";
        lines.push_back(ss.str());
        ss.str("");
    }
    
    if (_config.showCPU) {
        const auto& cpu = profiler.getCPUMetrics();
        ss << "CPU: " << std::fixed << std::setprecision(1) << cpu.cpuUsagePercent << "%";
        lines.push_back(ss.str());
        ss.str("");
    }
    
    if (_config.showNetwork) {
        const auto& net = profiler.getNetworkMetrics();
        if (net.packetsSent > 0 || net.packetsReceived > 0) {
            ss << "Latency: " << std::fixed << std::setprecision(1) << net.latency << "ms"
               << " (avg: " << net.avgLatency << "ms)";
            lines.push_back(ss.str());
            ss.str("");
            
            ss << "Packets: " << net.packetsSent << "/" << net.packetsReceived
               << " (dropped: " << net.packetsDropped << ")";
            lines.push_back(ss.str());
            ss.str("");
        }
    }
    
    if (_config.showWorld) {
        const auto& world = profiler.getWorldMetrics();
        if (world.entityCount > 0) {
            ss << "Entities: " << world.entityCount;
            lines.push_back(ss.str());
            ss.str("");
            
            ss << "Pos: (" << std::fixed << std::setprecision(1) 
               << world.positionX << ", " << world.positionY << ")";
            lines.push_back(ss.str());
            ss.str("");
        }
    }
    
    if (_config.showScopes) {
        const auto& scopes = profiler.getAllScopeTimes();
        if (!scopes.empty()) {
            for (const auto& [name, time] : scopes) {
                ss << name << ": " << std::fixed << std::setprecision(2) << time << "ms";
                lines.push_back(ss.str());
                ss.str("");
            }
        }
    }
    
    return lines;
}

}
}
