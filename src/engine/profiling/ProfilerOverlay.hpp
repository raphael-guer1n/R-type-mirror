#pragma once

#include "engine/profiling/Profiler.hpp"
#include <string>
#include <vector>

struct SDL_Renderer;
typedef void* TTF_FontPtr;

namespace Engine {
namespace Profiling {

struct ProfilerDisplayConfig {
    bool showFPS = true;
    bool showFrameTime = true;
    bool showMemory = true;
    bool showCPU = true;
    bool showNetwork = true;
    bool showWorld = true;
    bool showScopes = true;
    
    int posX = 10;
    int posY = 10;
    int fontSize = 14;
    
    struct Color {
        uint8_t r, g, b, a;
    };
    
    Color textColor = {255, 255, 255, 255};
    Color backgroundColor = {0, 0, 0, 180};
};

class ProfilerOverlay {
public:
    ProfilerOverlay();
    ~ProfilerOverlay();
    bool initialize(SDL_Renderer* renderer, const std::string& fontPath);
    void render();
    void setConfig(const ProfilerDisplayConfig& config) { _config = config; }
    const ProfilerDisplayConfig& getConfig() const { return _config; }
    void setVisible(bool visible) { _visible = visible; }
    bool isVisible() const { return _visible; }
    void toggleVisibility() { _visible = !_visible; }

private:
    void renderText(const std::string& text, int x, int y);
    void renderBackground(int x, int y, int width, int height);
    std::vector<std::string> formatMetrics();
    
    SDL_Renderer* _renderer = nullptr;
    TTF_FontPtr _font = nullptr;
    ProfilerDisplayConfig _config;
    bool _visible = true;
    bool _initialized = false;
};

}
}
