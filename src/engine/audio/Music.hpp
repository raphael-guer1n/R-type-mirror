/**
 * @file Music.hpp
 * @brief Definition of the Music class for handling audio playback in the engine.
 *
 * This file defines the `engine::audio::Music` class, which provides
 * a simple interface for loading and playing background music within
 * the R-Type engine. It serves as an abstraction layer over SDL_mixer,
 * simplifying the management of music playback and resource cleanup.
 *
 * @details
 * The class supports:
 * - Loading music files (e.g., `.ogg`, `.mp3`, `.wav`) from a given file path.
 * - Playing the loaded track, either once or in a loop.
 * - Stopping playback and freeing associated resources.
 *
 * Internally, it uses the **SDL_mixer** library and manages a `Mix_Music*` handle.
 *
 * ### Example usage:
 * @code
 * engine::audio::Music bgMusic;
 * if (bgMusic.load("./Assets/Music/Game.ogg")) {
 *     bgMusic.play(true); // loop enabled
 * }
 * @endcode
 *
 * @note Make sure SDL_mixer is initialized before creating Music objects.
 *
 * @namespace engine::audio
 * Namespace containing all audio-related utilities of the engine.
 *
 * @class engine::audio::Music
 * @brief Provides methods to load, play, and stop background music.
 *
 * @see Mix_Music
 * @see SDL_mixer
 */

#pragma once

#include <string>
#include <SDL_mixer.h>

namespace engine {

namespace audio {
    
    class Music {
    public:
        Music();
        ~Music();

        bool load(const std::string &path);
        void play(bool loop = true);
        void stop();

    private:
        Mix_Music *_music = nullptr;
    };

}} // namespace engine::audio
