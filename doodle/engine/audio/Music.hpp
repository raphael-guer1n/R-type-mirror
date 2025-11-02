/**
 * @file Music.hpp
 * @brief Declaration of the Music class, responsible for handling individual music or sound playback.
 *
 * This class wraps the low-level functionality of the Miniaudio library to provide
 * simple loading, playback, and control of music or sound effects within the R-Type engine.
 *
 * @note This class is designed for use by the @ref engine::audio::AudioManager,
 * but can also be used independently for standalone audio control.
 */

#pragma once

#include <string>
#include "miniaudio.h"

namespace engine {
namespace audio {

/**
 * @class Music
 * @brief Encapsulates an audio resource (music or sound effect) and provides playback control.
 *
 * The Music class provides a lightweight interface to load and control sound playback
 * using the Miniaudio library. It manages initialization, playback state, muting,
 * and resource cleanup automatically.
 *
 * Typical usage:
 * @code
 * engine::audio::Music bgm;
 * bgm.load("assets/audio/menu_theme.ogg");
 * bgm.play(true); // Loop music
 * @endcode
 *
 * @note Each Music instance manages its own Miniaudio `ma_sound` object.
 */
class Music {
public:
    /**
     * @brief Constructs a new Music object and initializes the audio engine.
     */
    Music();

    /**
     * @brief Destroys the Music object and releases associated audio resources.
     */
    ~Music();

    /**
     * @brief Loads a music or sound file from the given path.
     *
     * Supported formats depend on the Miniaudio backend (e.g., WAV, OGG, MP3, FLAC).
     *
     * @param path Path to the audio file to load.
     * @return True if the file was successfully loaded, false otherwise.
     */
    bool load(const std::string& path);

    /**
     * @brief Plays the loaded audio track.
     *
     * If no track has been loaded, this function does nothing.
     *
     * @param loop Whether the sound should loop continuously (default: true).
     */
    void play(bool loop = true);

    /**
     * @brief Stops playback of the current track.
     */
    void stop();

    /**
     * @brief Pauses playback of the current track.
     */
    void pause();

    /**
     * @brief Resumes playback of a paused track.
     */
    void resume();

    /**
     * @brief Checks whether the sound is currently muted.
     *
     * @return True if muted, false otherwise.
     */
    bool isMuted() const { return _muted; }

    /**
     * @brief Enables or disables muting for this track.
     *
     * @param muted True to mute the sound, false to unmute.
     */
    void setMuted(bool muted) { _muted = muted; }

private:
    /** @brief Miniaudio engine instance used for playback. */
    ma_engine _engine;

    /** @brief Miniaudio sound object representing the loaded audio resource. */
    ma_sound _sound;

    /** @brief Indicates whether a sound file has been successfully loaded. */
    bool _isLoaded = false;

    /** @brief Indicates whether the sound is currently muted. */
    bool _muted = false;
};

} // namespace audio
} // namespace engine
