#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Music.hpp"
#include <nlohmann/json.hpp>

namespace engine::audio {

class AudioManager {
public:
    static AudioManager& instance();

    bool loadConfig(const std::string& configPath);

    void playMusic(const std::string& name, bool loop = true);
    void stopMusic();
    void playSound(const std::string& name);

    void setMusicVolume(float volume);
    void setSFXVolume(float volume);

    float getMusicVolume() const { return _musicVolume; }
    float getSFXVolume() const { return _sfxVolume; }

private:
    AudioManager() = default;
    ~AudioManager() = default;
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    std::unordered_map<std::string, std::unique_ptr<Music>> _musics;
    std::unordered_map<std::string, std::unique_ptr<Music>> _sounds;

    std::string _currentMusic;
    float _musicVolume = 1.0f;
    float _sfxVolume = 1.0f;
};

}
