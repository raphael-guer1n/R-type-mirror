#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "engine/events/Events.hpp"

struct AccessibilityConfig {
    static bool enabled;
    static bool contrast_mode;
    static std::string mode_daltonien;
    static bool flash_effect;
    static float taille_hud;

    static bool subtitles;
    static bool indicateurs_visuals_audio;
    static float volume_general;

    static std::unordered_map<std::string, std::string> key_remap;
    static bool auto_fire;

    static bool less_movement;
    static float speed_game;

    static void load_from_json(const std::string &path);
};
engine::R_Events::Key stringToKey(const std::string &keyName);
