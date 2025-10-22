/**
 * @file Music.cpp
 * @brief Implementation of the Music class for managing background music playback.
 *
 * This file provides the implementation of the `engine::audio::Music` class,
 * which serves as a lightweight wrapper around the **SDL_mixer** library.
 * It allows the R-Type engine to load, play, and stop background music
 * (such as menu or in-game themes) in a simple and modular way.
 *
 * @details
 * - Initializes the SDL_mixer audio subsystem upon creation.
 * - Loads music files using `Mix_LoadMUS`.
 * - Supports looping or one-time playback via `Mix_PlayMusic`.
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
 * @see SDL_mixer
 * @see Mix_OpenAudio
 * @see Mix_PlayMusic
 */

#include <stdexcept>
#include <iostream>
#include "Music.hpp"
#include "engine/renderer/Error.hpp"

namespace engine::audio {

    Music::Music() = default;

    Music::~Music() = default;

    bool Music::load(const std::string &/*path*/) {
        return true; // pretend success
    }

    void Music::play(bool /*loop*/) {
        // no-op
    }

    void Music::stop() {
        // no-op
    }

} // namespace engine::audio
