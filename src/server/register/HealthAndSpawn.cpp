#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "common/Systems.hpp"
#include "server/GameLogic.hpp"

void GameLogic::register_health_and_spawn_systems()
{
  _registry.add_system<component::health, component::damage>(health_system);
  _registry.add_system<component::spawn_request>(spawn_system);
}
