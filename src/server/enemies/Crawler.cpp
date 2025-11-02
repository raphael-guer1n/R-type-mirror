#include <iostream>
#include "server/Server.hpp"
#include "Crawler.hpp"
#include "server/EnemyConfig.hpp"

void Enemies::Crawler::NewCrawler(engine::registry &reg,
    std::unordered_set<uint32_t>& entities, std::mt19937& gen)
{
    try
    {
        EnemyConfig cfg = EnemyConfig::load_enemy_config("configs/enemy/crawler.json");
        auto e = reg.spawn_entity();
        entities.insert((uint32_t)e);

        reg.add_component(e, component::position{SCREEN_WIDTH + 100,
            static_cast<float>(std::uniform_int_distribution<int>
                (100, 1000)(gen))});
        reg.add_component(e, component::velocity{0, 0});
        reg.add_component<component::hitbox>(e, std::move(cfg.hitbox));
        reg.add_component(e, component::entity_kind::enemy);
        reg.add_component(e, component::collision_state{false});
        reg.add_component(e, component::health{(uint8_t)cfg.hp});

        component::ai_controller ai;
        ai.behavior = cfg.behavior;
        ai.speed = cfg.speed;
        reg.add_component<component::ai_controller>(e, std::move(ai));
        if (!cfg.spells.empty())
        {
            component::spellbook sb;
            sb.spells = cfg.spells;
            reg.add_component<component::spellbook>(e, std::move(sb));
        }
    }
    catch (std::exception &ex)
    {
        std::cerr << "Failed to load zigzag enemy: " << ex.what() << "\n";
    }
}
