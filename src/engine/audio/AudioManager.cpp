#include "AudioManager.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace engine::audio {

AudioManager& AudioManager::instance() {
    static AudioManager _instance;
    return _instance;
}

bool AudioManager::loadConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << " Failed to open config: " << configPath << std::endl;
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << " JSON parse error: " << e.what() << std::endl;
        return false;
    }

    if (j.contains("music")) {
        for (auto& [name, path] : j["music"].items()) {
            auto music = std::make_unique<Music>();
            if (music->load(path)) {
                _musics[name] = std::move(music);
                std::cout << "Loaded music: " << name << std::endl;
            }
        }
    }

    if (j.contains("sounds")) {
        for (auto& [name, path] : j["sounds"].items()) {
            auto sound = std::make_unique<Music>();
            if (sound->load(path)) {
                _sounds[name] = std::move(sound);
                std::cout << "Loaded sound: " << name << std::endl;
            }
        }
    }

    if (j.contains("volumes")) {
        if (j["volumes"].contains("music"))
            _musicVolume = j["volumes"]["music"];
        if (j["volumes"].contains("sfx"))
            _sfxVolume = j["volumes"]["sfx"];
    }

    return true;
}

void AudioManager::playMusic(const std::string& name, bool loop) {
    auto it = _musics.find(name);
    if (it == _musics.end()) {
        std::cerr << "Music not found: " << name << std::endl;
        return;
    }

    if (!_currentMusic.empty() && _musics[_currentMusic])
        _musics[_currentMusic]->stop();

    _currentMusic = name;
    it->second->play(loop);
}

void AudioManager::stopMusic() {
    if (!_currentMusic.empty() && _musics[_currentMusic])
        _musics[_currentMusic]->stop();
}

void AudioManager::playSound(const std::string& name) {
    auto it = _sounds.find(name);
    if (it == _sounds.end()) {
        std::cerr << "Sound not found: " << name << std::endl;
        return;
    }
    it->second->play(false);
}

void AudioManager::setMusicVolume(float volume) {
    _musicVolume = volume;
}

void AudioManager::setSFXVolume(float volume) {
    _sfxVolume = volume;
}

} // namespace engine::audio
