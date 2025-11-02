/**
 * @file AudioManager.hpp
 * @brief Declaration of the AudioManager class responsible for handling all audio playback in the engine.
 *
 * The AudioManager class provides a centralized interface for managing background music
 * and sound effects within the R-Type engine. It supports loading audio configurations,
 * playing or stopping sounds and music, and adjusting volume levels globally.
 *
 * @note This class is implemented as a singleton, accessible via @ref AudioManager::instance().
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "Music.hpp"
#include <nlohmann/json.hpp>

namespace engine::audio {

/**
 * @class AudioManager
 * @brief Manages background music and sound effects for the R-Type engine.
 *
 * The AudioManager is a singleton class that loads, plays, and controls all game audio.
 * It maintains a map of loaded music and sound resources, manages volume levels,
 * and allows switching or looping music tracks dynamically.
 *
 * @note This class wraps functionality provided by the underlying audio engine (miniaudio via `ma_engine`).
 */
class AudioManager {
public:
    /**
     * @brief Provides access to the global AudioManager instance (singleton).
     *
     * @return Reference to the singleton AudioManager instance.
     */
    static AudioManager& instance();

    /**
     * @brief Loads an audio configuration file (JSON format).
     *
     * Reads paths to music and sound assets from a JSON file and loads them into memory.
     * The JSON file should contain separate entries for music and sound effects.
     *
     * @param configPath Path to the JSON configuration file.
     * @return True if the configuration was successfully loaded, false otherwise.
     *
     * @throws std::runtime_error if the file cannot be opened or parsed.
     */
    bool loadConfig(const std::string& configPath);

    /**
     * @brief Plays a background music track by name.
     *
     * If another track is already playing, it will be stopped before starting the new one.
     *
     * @param name Name of the music track to play (as defined in the config file).
     * @param loop Whether the track should loop continuously (default: true).
     */
    void playMusic(const std::string& name, bool loop = true);

    /**
     * @brief Stops the currently playing background music.
     */
    void stopMusic();

    /**
     * @brief Plays a sound effect by name.
     *
     * Sound effects are non-looping and can be played simultaneously with music.
     *
     * @param name Name of the sound effect to play (as defined in the config file).
     */
    void playSound(const std::string& name);

    /**
     * @brief Sets the volume level for background music.
     *
     * @param volume Volume level in the range [0.0f, 1.0f].
     */
    void setMusicVolume(float volume);

    /**
     * @brief Sets the volume level for sound effects (SFX).
     *
     * @param volume Volume level in the range [0.0f, 1.0f].
     */
    void setSFXVolume(float volume);

    /**
     * @brief Retrieves the current music volume.
     *
     * @return The current music volume in the range [0.0f, 1.0f].
     */
    float getMusicVolume() const { return _musicVolume; }

    /**
     * @brief Retrieves the current sound effects (SFX) volume.
     *
     * @return The current SFX volume in the range [0.0f, 1.0f].
     */
    float getSFXVolume() const { return _sfxVolume; }

private:
    /**
     * @brief Private default constructor to enforce the singleton pattern.
     */
    AudioManager() = default;

    /**
     * @brief Private destructor.
     */
    ~AudioManager() = default;

    /**
     * @brief Deleted copy constructor (singleton non-copyable).
     */
    AudioManager(const AudioManager&) = delete;

    /**
     * @brief Deleted assignment operator (singleton non-assignable).
     */
    AudioManager& operator=(const AudioManager&) = delete;

    /** @name Audio Resources */
    ///@{
    std::unordered_map<std::string, std::unique_ptr<Music>> _musics;  /**< Loaded music tracks. */
    std::unordered_map<std::string, std::unique_ptr<Music>> _sounds;  /**< Loaded sound effects. */
    ///@}

    /** @brief Name of the currently playing music track. */
    std::string _currentMusic;

    /** @brief Current music volume level. */
    float _musicVolume = 1.0f;

    /** @brief Current sound effects (SFX) volume level. */
    float _sfxVolume = 1.0f;

    /** @brief Underlying audio engine instance (Miniaudio). */
    ma_engine _engine;
};

} // namespace engine::audio
