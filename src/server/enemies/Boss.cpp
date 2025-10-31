#include <iostream>
#include "Shooter.hpp"
#include "server/EnemyConfig.hpp"
#include "server/Server.hpp"
#include "common/Components.hpp"
#include "Boss.hpp"

void Enemies::Boss::NewBoss(engine::registry &reg,
    std::unordered_set<uint32_t> &entities, std::mt19937 &gen)
{
    try
    {
        EnemyConfig cfg = EnemyConfig::load_enemy_config("configs/enemy/boss.json");
        auto boss = reg.spawn_entity();
        entities.insert(static_cast<uint32_t>(boss));

        reg.add_component(boss, component::position{1800.f, 400.f});
        reg.add_component(boss, component::velocity{0.f, 0.f});
        reg.add_component<component::hitbox>(boss, std::move(cfg.hitbox));
        reg.add_component(boss, component::entity_kind::enemy);
        reg.add_component(boss, component::collision_state{false});
        reg.add_component(boss, component::health{(uint8_t)cfg.hp});

        component::ai_controller ai;
        ai.behavior = cfg.behavior; // "boss"
        ai.speed = cfg.speed;
        reg.add_component<component::ai_controller>(boss, std::move(ai));

        if (!cfg.spells.empty())
        {
            component::spellbook sb;
            sb.spells = cfg.spells;
            reg.add_component<component::spellbook>(boss, std::move(sb));
        }

        std::cout << "Boss spawned! HP: " << cfg.hp << " Behavior: " << ai.behavior << "\n";
    }
    catch (std::exception &ex)
    {
        std::cerr << "Failed to spawn boss: " << ex.what() << "\n";
    }
}