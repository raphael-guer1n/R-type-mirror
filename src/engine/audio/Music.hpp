/**
 * @file Music.hpp
 * @brief Definition of the Music class for handling audio playback in the engine.
 *
 * This file defines the `engine::audio::Music` class, which provides
 * a simple interface for loading and playing background music within
 * the R-Type engine. It serves as an abstraction layer over miniaudio,
 * simplifying the management of music playback and resource cleanup.
 *
 * @details
 * The class supports:
 * - Loading music files (e.g., `.ogg`, `.mp3`, `.wav`) from a given file path.
 * - Playing the loaded track, either once or in a loop.
 * - Stopping playback and freeing associated resources.
 * - Pausing and resuming playback.
 * - Muting functionality.
 *
 * Internally, it uses the **miniaudio** library which works on Fedora 42.
 *
 * ### Example usage:
 * @code
 * engine::audio::Music bgMusic;
 * if (bgMusic.load("./Assets/Music/Game.ogg")) {
 *     bgMusic.play(true); // loop enabled
 * }
 * @endcode
 *
 * @namespace engine::audio
 * Namespace containing all audio-related utilities of the engine.
 *
 * @class engine::audio::Music
 * @brief Provides methods to load, play, and stop background music.
 *
 * @see miniaudio
 */

#pragma once

#include <string>
#include "miniaudio.h"

namespace engine {
namespace audio {

class Music {
public:
    Music();
    ~Music();

    bool load(const std::string& path);
    void play(bool loop = true);
    void stop();
    void pause();
    void resume();
    bool isMuted() const { return _muted; }
    void setMuted(bool muted) { _muted = muted; }

private:
    ma_engine _engine;
    ma_sound _sound;
    bool _isLoaded = false;
    bool _muted = false;
};

} // namespace audio
} // namespace engine
