#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include "common/Components_client.hpp"
#include "server/Components_ai.hpp"
#include "common/Components.hpp"

using json = nlohmann::json;

class EnemyConfig
{
public:
    std::string type;
    int hp{1};
    float speed{1.0f};
    std::string behavior{"basic"};
    component::hitbox hitbox;
    std::vector<component::spell> spells;

    static EnemyConfig load_enemy_config(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
            throw std::runtime_error("Could not open config file: " + path);

        json j;
        file >> j;

        EnemyConfig cfg;
        cfg.type = j.value("type", "enemy");
        cfg.hp = j.value("hp", 1);
        cfg.speed = j.value("speed", 1.0f);
        cfg.behavior = j.value("behavior", "basic");

        if (j.contains("hitbox"))
        {
            cfg.hitbox = component::hitbox(
                j["hitbox"].value("w", 20.f),
                j["hitbox"].value("h", 20.f));
        }
        if (j.contains("spells"))
        {
            for (auto &s : j["spells"])
            {
                component::spell sp;
                sp.name = s.value("name", "");
                sp.cooldown = s.value("cooldown", 60);
                sp.damage = s.value("damage", 1);
                sp.speedX = s.value("speedX", -4.0f);
                sp.speedY = s.value("speedY", 0.0f);
                sp.width = s.value("width", 8.0f);
                sp.height = s.value("height", 8.0f);
                cfg.spells.push_back(sp);
            }
        }
        return cfg;
    }
};