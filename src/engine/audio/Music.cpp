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

#include "Music.hpp"
#include <stdexcept>
#include <iostream>

namespace engine::audio {

    Music::Music() {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            throw std::runtime_error(std::string("SDL_mixer error: ") + Mix_GetError());
        }
    }

    Music::~Music() {
        if (_music)
            Mix_FreeMusic(_music);
        Mix_CloseAudio();
    }

    bool Music::load(const std::string &path) {
        _music = Mix_LoadMUS(path.c_str());
        if (!_music) {
            std::cerr << "Failed to load music: " << path << " (" << Mix_GetError() << ")" << std::endl;
            return false;
        }
        return true;
    }

    void Music::play(bool loop) {
        if (!_music) return;
        Mix_PlayMusic(_music, loop ? -1 : 1);
    }

    void Music::stop() {
        Mix_HaltMusic();
    }

} // namespace engine::audio
