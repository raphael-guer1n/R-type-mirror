#include "common/Accessibility.hpp"
#include <fstream>
#include <iostream>

// === Définitions par défaut ===
bool AccessibilityConfig::enabled = false;

// --- Visuel ---
bool AccessibilityConfig::contrast_mode = false;
std::string AccessibilityConfig::mode_daltonien = "none";
bool AccessibilityConfig::flash_effect = true;
float AccessibilityConfig::taille_hud = 1.0f;

// --- Audio ---
bool AccessibilityConfig::subtitles = false;
bool AccessibilityConfig::indicateurs_visuals_audio = false;
float AccessibilityConfig::volume_general = 1.0f;

// --- Entrées ---
std::unordered_map<std::string, int> AccessibilityConfig::keyBindings = {
    {"move_up",    SDLK_UP},
    {"move_down",  SDLK_DOWN},
    {"move_left",  SDLK_LEFT},
    {"move_right", SDLK_RIGHT},
    {"shoot",      SDLK_SPACE},
    {"special",    SDLK_RSHIFT},
    {"pause",      SDLK_ESCAPE}
};

bool AccessibilityConfig::auto_fire = false;

// --- Cognitif ---
bool AccessibilityConfig::less_movement = false;
float AccessibilityConfig::speed_game = 1.0f;

// === Chargement du fichier JSON ===
void AccessibilityConfig::load_from_json(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Accessibility] Cannot open " << path << ", using default values.\n";
        return;
    }

    try {
        nlohmann::json j;
        file >> j;

        // VISUEL
        if (j.contains("visual")) {
            auto v = j["visual"];
            if (v.contains("contrast_mode")) contrast_mode = v["contrast_mode"].get<bool>();
            if (v.contains("mode_daltonien")) mode_daltonien = v["mode_daltonien"].get<std::string>();
            if (v.contains("flash_effect")) flash_effect = v["flash_effect"].get<bool>();
            if (v.contains("taille_hud")) taille_hud = v["taille_hud"].get<float>();
        }

        // AUDIO
        if (j.contains("audio")) {
            auto a = j["audio"];
            if (a.contains("subtitles")) subtitles = a["subtitles"].get<bool>();
            if (a.contains("indicateurs_visuals_audio")) indicateurs_visuals_audio = a["indicateurs_visuals_audio"].get<bool>();
            if (a.contains("volume_general")) volume_general = a["volume_general"].get<float>();
        }

        // ENTRÉES
        if (j.contains("enter")) {
            auto e = j["enter"];
            if (e.contains("auto_shoot")) auto_fire = e["auto_shoot"].get<bool>();

            if (e.contains("key_remap")) {
                for (auto &[action, keyName] : e["key_remap"].items()) {
                    std::string k = keyName.get<std::string>();
                    AccessibilityConfig::keyBindings[action] = static_cast<int>(stringToKey(k));
                }
            }
        }

        // COGNITIF
        if (j.contains("cognitive")) {
            auto c = j["cognitive"];
            if (c.contains("less_movement")) less_movement = c["less_movement"].get<bool>();
            if (c.contains("speed_game")) speed_game = c["speed_game"].get<float>();
        }

        enabled = true;
        std::cout << "[Accessibility] Config loaded successfully from " << path << "\n";

    } catch (std::exception &e) {
        std::cerr << "[Accessibility] JSON parsing error: " << e.what() << "\n";
    }
}

// === Sauvegarde du JSON (si tu modifies les touches en jeu) ===
void AccessibilityConfig::save_to_json(const std::string &path)
{
    nlohmann::json j;
    j["visual"] = {
        {"contrast_mode", contrast_mode},
        {"mode_daltonien", mode_daltonien},
        {"flash_effect", flash_effect},
        {"taille_hud", taille_hud}
    };
    j["audio"] = {
        {"subtitles", subtitles},
        {"indicateurs_visuals_audio", indicateurs_visuals_audio},
        {"volume_general", volume_general}
    };
    j["enter"]["auto_shoot"] = auto_fire;

    for (auto &[action, key] : keyBindings) {
        j["enter"]["key_remap"][action] = std::to_string(key);
    }

    j["cognitive"] = {
        {"less_movement", less_movement},
        {"speed_game", speed_game}
    };

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Accessibility] Failed to save config to " << path << "\n";
        return;
    }
    file << j.dump(4);
    std::cout << "[Accessibility] Config saved to " << path << "\n";
}

#include "engine/events/Events.hpp"
#include <SDL.h>
#include <unordered_map>
#include <algorithm>
#include <iostream>

engine::R_Events::Key stringToKey(const std::string &keyName)
{
    static const std::unordered_map<std::string, engine::R_Events::Key> keyMap = {
        {"up", engine::R_Events::Key::Up},
        {"down", engine::R_Events::Key::Down},
        {"left", engine::R_Events::Key::Left},
        {"right", engine::R_Events::Key::Right},
        {"space", engine::R_Events::Key::Space},
        {"rshift", engine::R_Events::Key::RShift},
        {"lshift", engine::R_Events::Key::LShift},
        {"escape", engine::R_Events::Key::Escape},
        {"a", engine::R_Events::Key::A},
        {"b", engine::R_Events::Key::B},
        {"c", engine::R_Events::Key::C},
        {"d", engine::R_Events::Key::D},
        {"e", engine::R_Events::Key::E},
        {"f", engine::R_Events::Key::F},
        {"g", engine::R_Events::Key::G},
        {"h", engine::R_Events::Key::H},
        {"i", engine::R_Events::Key::I},
        {"j", engine::R_Events::Key::J},
        {"k", engine::R_Events::Key::K},
        {"l", engine::R_Events::Key::L},
        {"m", engine::R_Events::Key::M},
        {"n", engine::R_Events::Key::N},
        {"o", engine::R_Events::Key::O},
        {"p", engine::R_Events::Key::P},
        {"q", engine::R_Events::Key::Q},
        {"r", engine::R_Events::Key::R},
        {"s", engine::R_Events::Key::S},
        {"t", engine::R_Events::Key::T},
        {"u", engine::R_Events::Key::U},
        {"v", engine::R_Events::Key::V},
        {"w", engine::R_Events::Key::W},
        {"x", engine::R_Events::Key::X},
        {"y", engine::R_Events::Key::Y},
        {"z", engine::R_Events::Key::Z}
    };

    std::string lower = keyName;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (keyMap.count(lower))
        return keyMap.at(lower);

    std::cerr << "[Accessibility] Unknown key: " << keyName << ", defaulting to Space.\n";
    return engine::R_Events::Key::Space;
}