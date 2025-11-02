/**
 * @file Music.cpp
 * @brief Implementation of the Music class for managing background music playback.
 *
 * This file provides the implementation of the `engine::audio::Music` class,
 * which serves as a lightweight wrapper around the **miniaudio** library.
 * It allows the R-Type engine to load, play, and stop background music
 * (such as menu or in-game themes) in a simple and modular way.
 *
 * @details
 * - Initializes the miniaudio engine upon creation.
 * - Loads music files using `ma_sound_init_from_file`.
 * - Supports looping or one-time playback via `ma_sound_start`.
 * - Stops and cleans up resources automatically when destroyed.
 * - Throws a `std::runtime_error` if audio initialization fails.
 *
 * ### Example usage:
 * @code
 * try {
 *     engine::audio::Music music;
 *     if (music.load("./Assets/Music/Game.ogg")) {
 *         music.play(true);  // Play in loop
 *     }
 * } catch (const std::exception &e) {
 *     std::cerr << e.what() << std::endl;
 * }
 * @endcode
 *
 * @see engine::audio::Music
 * @see miniaudio
 */
#define MINIAUDIO_IMPLEMENTATION
#include "Music.hpp"
#include <iostream>
#include <stdexcept>

namespace engine::audio {

Music::Music() {
    ma_result result = ma_engine_init(nullptr, &_engine);
    if (result != MA_SUCCESS) {
        std::cerr << " Failed to initialize Miniaudio engine (error " << result << ")." << std::endl;
        throw std::runtime_error("Failed to initialize Miniaudio engine");
    }
    _isLoaded = false;
}

Music::~Music() {
    if (_isLoaded) {
        ma_sound_uninit(&_sound);
    }
    ma_engine_uninit(&_engine);
}

bool Music::load(const std::string& path) {
    ma_result result = ma_sound_init_from_file(&_engine, path.c_str(), 0, nullptr, nullptr, &_sound);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load sound: " << path << " (error " << result << ")" << std::endl;
        _isLoaded = false;
        return false;
    }
    _isLoaded = true;
    return true;
}

void Music::play(bool loop) {
    if (!_isLoaded || _muted) return;
    ma_sound_set_looping(&_sound, loop ? MA_TRUE : MA_FALSE);
    ma_sound_start(&_sound);
}

void Music::stop() {
    if (!_isLoaded) return;
    ma_sound_stop(&_sound);
}

void Music::pause() {
    if (!_isLoaded) return;
    ma_sound_stop(&_sound);
}

void Music::resume() {
    if (!_isLoaded || _muted) return;
    ma_sound_start(&_sound);
}

} // namespace engine::audio
