#include "common/Accessibility.hpp"
#include "engine/events/Events.hpp"
#include <fstream>
#include <iostream>

bool AccessibilityConfig::enabled = false;
bool AccessibilityConfig::contrast_mode = false;
std::string AccessibilityConfig::mode_daltonien = "none";
bool AccessibilityConfig::flash_effect = true;
float AccessibilityConfig::taille_hud = 1.0f;

bool AccessibilityConfig::subtitles = false;
bool AccessibilityConfig::indicateurs_visuals_audio = false;
float AccessibilityConfig::volume_general = 1.0f;

std::unordered_map<std::string, std::string> AccessibilityConfig::key_remap = {
    {"tir", "SPACE"},
    {"tir_alternatif", "C"},
    {"pause", "ESCAPE"}};
bool AccessibilityConfig::auto_fire = false;

bool AccessibilityConfig::less_movement = false;
float AccessibilityConfig::speed_game = 1.0f;

void AccessibilityConfig::load_from_json(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Accessibility] cannot open " << path << ", using default value.\n";
        return;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (std::exception &e) {
        std::cerr << "[Accessibility] json parsing error : " << e.what() << "\n";
        return;
    }
    if (j.contains("visual")) {
        auto v = j["visual"];
        if (v.contains("contrast_mode")) contrast_mode = v["contrast_mode"].get<bool>();
        if (v.contains("mode_daltonien")) mode_daltonien = v["mode_daltonien"].get<std::string>();
        if (v.contains("flash_effect")) flash_effect = v["flash_effect"].get<bool>();
        if (v.contains("taille_hud")) taille_hud = v["taille_hud"].get<float>();
    }
    if (j.contains("audio")) {
        auto a = j["audio"];
        if (a.contains("subtitles")) subtitles = a["subtitles"].get<bool>();
        if (a.contains("indicateurs_visuals_audio")) indicateurs_visuals_audio = a["indicateurs_visuals_audio"].get<bool>();
        if (a.contains("volume_general")) volume_general = a["volume_general"].get<float>();
    }
    if (j.contains("enter")) {
        auto e = j["enter"];
        if (e.contains("key_remap")) key_remap = e["key_remap"].get<std::unordered_map<std::string, std::string>>();
        if (e.contains("auto_shoot")) auto_fire = e["auto_shoot"].get<bool>();
    }

    if (j.contains("cognitive")) {
        auto c = j["cognitive"];
        if (c.contains("less_movement")) less_movement = c["less_movement"].get<bool>();
        if (c.contains("speed_game")) speed_game = c["speed_game"].get<float>();
    }
    enabled = true;

}

static std::string up(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

engine::R_Events::Key stringToKey(const std::string &name)
{
    const std::string n = up(name);

    if (n == "LEFT") return engine::R_Events::Key::Left;
    if (n == "RIGHT") return engine::R_Events::Key::Right;
    if (n == "UP") return engine::R_Events::Key::Up;
    if (n == "DOWN") return engine::R_Events::Key::Down;

    if (n == "SPACE") return engine::R_Events::Key::Space;
    if (n == "RETURN" || n == "ENTER") return engine::R_Events::Key::Enter;
    if (n == "ESCAPE" || n == "ESC") return engine::R_Events::Key::Escape;

    if (n == "A") return engine::R_Events::Key::A;
    if (n == "B") return engine::R_Events::Key::B;
    if (n == "C") return engine::R_Events::Key::C;
    if (n == "RSHIFT") return engine::R_Events::Key::RShift;
    if (n == "LSHIFT") return engine::R_Events::Key::LShift;

    return engine::R_Events::Key::Unknown;
}