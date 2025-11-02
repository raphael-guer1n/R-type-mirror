#pragma once
#include <unordered_set>
#include <random>
#include "engine/ecs/Registry.hpp"

namespace Enemies
{
    class Shooter
    {
        public:
            static void NewShooter(engine::registry& reg,
                std::unordered_set<uint32_t>& entities, std::mt19937& gen);
        public:
            ~Shooter() = default;
        private:
            Shooter() = default;
    };
}