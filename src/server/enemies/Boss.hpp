#pragma once
#include <unordered_set>
#include <random>
#include "engine/ecs/Registry.hpp"

namespace Enemies
{
    class Boss
    {
        public:
            static void NewBoss(engine::registry &reg,
            std::unordered_set<uint32_t> &entities, std::mt19937 &gen);
        public:
            ~Boss() = default;

        private:
            Boss() = default;
    };
}