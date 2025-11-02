/**
 * @file AccessibilityConfig.hpp
 * @brief Declaration of the AccessibilityConfig struct for managing accessibility options and settings.
 *
 * This file defines configuration settings that improve accessibility in the R-Type game.
 * It supports JSON-based loading and saving of accessibility preferences such as high contrast,
 * color-blind modes, subtitles, and key bindings.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "engine/events/Events.hpp"

/**
 * @struct AccessibilityConfig
 * @brief Contains configuration settings related to accessibility and user assistance features.
 *
 * The AccessibilityConfig struct stores and manages all accessibility-related options for the game,
 * including visual, auditory, and gameplay adjustments. It can serialize and deserialize its settings
 * to and from JSON files to preserve user preferences.
 *
 * @note All members and methods are static, meaning the configuration is shared across the entire game.
 */
struct AccessibilityConfig {
    /** @brief Global toggle for enabling or disabling accessibility features. */
    static bool enabled;

    /** @name Visual Accessibility Settings */
    ///@{
    static bool contrast_mode;                  /**< Enables high-contrast mode for improved readability. */
    static std::string mode_daltonien;          /**< Sets the color-blind mode type (e.g., "protanopia", "deuteranopia"). */
    static bool flash_effect;                   /**< Enables or disables flash/strobe effects. */
    static float taille_hud;                    /**< Adjusts HUD (Heads-Up Display) size for readability. */
    ///@}

    /** @name Audio Accessibility Settings */
    ///@{
    static bool subtitles;                      /**< Enables subtitles for in-game dialogue or sound cues. */
    static bool indicateurs_visuals_audio;      /**< Enables visual indicators for sound direction or volume. */
    static float volume_general;                /**< Master volume level for accessibility purposes. */
    ///@}

    /** @name Gameplay Accessibility Settings */
    ///@{
    static std::unordered_map<std::string, int> keyBindings;  /**< Custom key bindings for accessible controls. */
    static bool auto_fire;                      /**< Enables automatic firing for players with reduced mobility. */
    static bool less_movement;                  /**< Reduces camera or world movement for motion sensitivity. */
    static float speed_game;                    /**< Adjusts overall game speed for accessibility. */
    ///@}

    /**
     * @brief Loads accessibility configuration from a JSON file.
     *
     * Reads settings from the specified JSON file and updates the configuration accordingly.
     *
     * @param path Path to the JSON configuration file.
     *
     * @throws std::runtime_error if the file cannot be opened or parsed.
     */
    static void load_from_json(const std::string &path);

    /**
     * @brief Saves the current accessibility configuration to a JSON file.
     *
     * Serializes the current settings to the specified file for later retrieval.
     *
     * @param path Path to the JSON file where the configuration will be saved.
     *
     * @throws std::runtime_error if the file cannot be created or written to.
     */
    static void save_to_json(const std::string &path);
};

/**
 * @brief Converts a string representation of a key to the corresponding engine key code.
 *
 * This utility function translates human-readable key names (e.g., `"Space"`, `"Enter"`, `"A"`)
 * into their corresponding @ref engine::R_Events::Key enumeration values.
 *
 * @param keyName The string name of the key to convert.
 * @return The corresponding @ref engine::R_Events::Key value.
 */
engine::R_Events::Key stringToKey(const std::string &keyName);
