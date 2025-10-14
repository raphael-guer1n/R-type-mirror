#include <iostream>
#include <SDL.h>
#include "Rtype.hpp"
#include "Background.hpp"
#include "common/Components.hpp"
#include "common/Components_client.hpp"
#include "common/Components_client_sdl.hpp"
#include "common/Packets.hpp"
#include "engine/network/UdpSocket.hpp"
#include "common/Systems.hpp"
#include "Background.hpp"
#include "Hud.hpp"
#include "common/Systems_client_sdl.hpp"
#include "common/Layers.hpp"
#include "engine/renderer/Error.hpp"

R_Type::Rtype::Rtype()
    : _app("R-Type", 1920, 1080)
{
  try
  {
    _client = std::make_unique<engine::net::UdpSocket>(_ioContext, 0);
    _serverEndpoint = std::make_unique<engine::net::Endpoint>(engine::net::make_endpoint("127.0.0.1", 4242));

    _registry.register_component<component::drawable>();
    _registry.register_component<component::position>();
    _registry.register_component<component::velocity>();
    _registry.register_component<component::controllable>();
    _registry.register_component<component::entity_kind>();
    _registry.register_component<component::collision_state>();
    _registry.register_component<component::animation>();
    _registry.register_component<component::lifetime>();
    _registry.register_component<component::hitbox>();
    _background = std::make_unique<Background>(*this);
    _playerData = std::make_unique<Player>(*this);
    _enemyData = std::make_unique<Enemy>(*this);
    _hud = std::make_unique<Hud>(*this);
    _menu = std::make_unique<R_Type::Menu>(_app);
    _gameOverScreen = std::make_unique<Gameover>(_app);
  }
  catch (const engine::Error &e)
  {
    throw engine::Error(e);
  }
}

void R_Type::Rtype::update(float deltaTime,
                           const std::vector<R_Events::Event> &events)
{
  if (_gameOver)
    return;
  if (_inMenu)
  {
    bool start = _menu->update(events);
    if (start)
    {
      _inMenu = false;
      ConnectReq req{42};
      PacketHeader hdr{CONNECT_REQ, sizeof(ConnectReq), 0};
      std::vector<uint8_t> buf(sizeof(ConnectReq));
      std::memcpy(buf.data(), &req, sizeof(ConnectReq));
      _client->send(hdr, buf, *_serverEndpoint);
      std::cout << "Sent CONNECT_REQ\n";
    }
    return;
  }
  if (!_connected && !_inMenu) {
    waiting_connection();
    return;
  }
  for (auto &ev : events)
  {
    if (ev.type == R_Events::Type::KeyDown) {
      _pressedKeys.insert(ev.key.code);
    }
    else if (ev.type == R_Events::Type::KeyUp)
      _pressedKeys.erase(ev.key.code);
  }
  InputPacket inp{};
  inp.clientId = _player;
  inp.tick = _tick++;
  inp.keyCount = static_cast<uint16_t>(_pressedKeys.size());
  const uint16_t keyCount = inp.keyCount;
  std::vector<int32_t> keys;
  keys.reserve(keyCount);
  for (auto k : _pressedKeys)
    keys.push_back(static_cast<int32_t>(k));
  const uint16_t payloadSize = sizeof(InputPacket) + keyCount * sizeof(int32_t);
  PacketHeader ihdr{INPUT_PKT, payloadSize, _tick};
  std::vector<uint8_t> ibuf(payloadSize);
  std::memcpy(ibuf.data(), &inp, sizeof(InputPacket));
  if (keyCount > 0)
    std::memcpy(ibuf.data() + sizeof(InputPacket), keys.data(), keyCount * sizeof(int32_t));
  _client->send(ihdr, ibuf, *_serverEndpoint);
  receiveSnapshot();
  _playerData->playerUpdateAnimation(_entityMap, _player, _registry, _pressedKeys);
  auto &positions = _registry.get_components<component::position>();
  auto &animations = _registry.get_components<component::animation>();
  auto &velocities = _registry.get_components<component::velocity>();
  auto &controls = _registry.get_components<component::controllable>();
  auto &kinds = _registry.get_components<component::entity_kind>();
  auto &drawables = _registry.get_components<component::drawable>();
  auto &collisions = _registry.get_components<component::collision_state>();
  auto &hitboxes = _registry.get_components<component::hitbox>();
  position_system(_registry, positions, velocities, deltaTime);
  control_system(_registry, velocities, controls);
  scroll_reset_system(_registry, positions, kinds, _app);
  animation_system(_registry, animations, drawables, deltaTime);
  hitbox_system(_registry, positions, hitboxes, [this](size_t i, size_t j) {
    this->handle_collision(_registry, i, j);
  });
  lifetime_system(_registry, deltaTime);
  _registry.run_systems();
}

void R_Type::Rtype::receiveSnapshot()
{
  while (auto pkt_opt = _client->receive(_sender))
  {
    auto [shdr, spayload] = *pkt_opt;

    if (shdr.type == GAME_OVER && spayload.size() >= sizeof(GameOverPayload)) {
      GameOverPayload go{};
      std::memcpy(&go, spayload.data(), sizeof(go));
      _gameOver = true;
      uint32_t winnerEntityId = go.winnerEntityId;
      if (_player == winnerEntityId)
        _won = true;
      else
        _won = false;
      continue;
    }
    if (shdr.type == SNAPSHOT && spayload.size() >= sizeof(Snapshot))
    {
      Snapshot snap{};
      std::memcpy(&snap, spayload.data(), sizeof(Snapshot));

      auto &positions = _registry.get_components<component::position>();
      auto &velocities = _registry.get_components<component::velocity>();
      auto &drawables = _registry.get_components<component::drawable>();
      auto &kinds = _registry.get_components<component::entity_kind>();
      auto &collisions = _registry.get_components<component::collision_state>();
      auto &animations = _registry.get_components<component::animation>();
      auto &hitboxes = _registry.get_components<component::hitbox>();

      std::unordered_set<uint32_t> newActive;

      size_t n = snap.entityCount;
      if (spayload.size() >= sizeof(Snapshot) + n * sizeof(EntityState))
      {
        auto *entities = reinterpret_cast<const EntityState *>(
            spayload.data() + sizeof(Snapshot));

        auto ensure_slot = [](auto &arr, std::size_t idx, auto &&value)
        {
          if (idx >= arr.size())
          {
            arr.insert_at(idx, std::forward<decltype(value)>(value));
          }
          else if (!arr[idx])
          {
            arr.insert_at(idx, std::forward<decltype(value)>(value));
          }
        };

        for (size_t i = 0; i < n; ++i)
        {
          const EntityState &es = entities[i];

          size_t idLocal;
          auto it = _entityMap.find(es.entityId);
          if (it == _entityMap.end())
          {
            idLocal = drawables.size();
            _entityMap[es.entityId] = idLocal;
          }
          else
          {
            idLocal = it->second;
          }
          if (idLocal < kinds.size() && kinds[idLocal] &&
              kinds[idLocal].value() == component::entity_kind::decor)
          {
            continue;
          }

          newActive.insert(idLocal);

          ensure_slot(positions, idLocal, component::position{});
          ensure_slot(velocities, idLocal, component::velocity{});
          ensure_slot(kinds, idLocal, component::entity_kind{});
          ensure_slot(collisions, idLocal, component::collision_state{});
          kinds[idLocal] = static_cast<component::entity_kind>(es.type);
          std::shared_ptr<R_Graphic::Texture> tex;
          R_Graphic::textureRect rect;
          component::animation anim;
          switch (kinds[idLocal].value())
          {
          case component::entity_kind::playerProjectile:
            anim = _playerData->projectileAnimation;
            tex = _playerData->texture;
            rect = _playerData->projectileRect;
            ensure_slot(hitboxes, idLocal, component::hitbox{100, 24});
            ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Projectiles});
            break;
          case component::entity_kind::player:
            anim = _playerData->playerAnimation;
            tex = _playerData->texture;
            rect = _playerData->playerRect;
            ensure_slot(hitboxes, idLocal, component::hitbox{34, 20});
            ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Players});
            break;
          case component::entity_kind::enemyProjectile:
            anim = _enemyData->projectileAnimation;
            tex = _enemyData->projectileTexture;
            rect = _enemyData->projectileRect;
            ensure_slot(hitboxes, idLocal, component::hitbox{60, 60});
            ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Projectiles});
            break;
          case component::entity_kind::enemy:
            tex = _enemyData->enemyTexture;
            rect = _enemyData->enemyRect;
            ensure_slot(hitboxes, idLocal, component::hitbox{152, 100});
            ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Enemies});
            break;
          default:
            tex = _playerData->texture;
            rect = _playerData->playerRect;
            ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Effects});
            break;
          }
          ensure_slot(animations, idLocal, anim);

          positions[idLocal]->x = es.x;
          positions[idLocal]->y = es.y;
          collisions[idLocal]->collided = (es.collided != 0);
        }
      }

      if (!_activeEntities.empty())
      {
        for (auto id : _activeEntities)
        {
          if (newActive.find(id) == newActive.end())
          {
            if (id == _player)
              continue;

            if (id < kinds.size() && kinds[id] &&
                kinds[id].value() == component::entity_kind::decor)
            {
              continue;
            }

            if (id < positions.size() && positions[id])
              positions[id].reset();
            if (id < velocities.size() && velocities[id])
              velocities[id].reset();
            if (id < drawables.size() && drawables[id])
              drawables[id].reset();
            if (id < kinds.size() && kinds[id])
              kinds[id].reset();
            if (id < collisions.size() && collisions[id])
              collisions[id].reset();
          }
        }
      }
      _activeEntities = std::move(newActive);
    }
  }
}

void R_Type::Rtype::draw()
{
  if (_inMenu)
  {
    _menu->draw();
    return;
  }
  if (!_connected)
    return;
  if (_gameOver) {
    _gameOverScreen->draw(_won);
    return;
  }
  auto &positions = _registry.get_components<component::position>();
  auto &drawables = _registry.get_components<component::drawable>();

  draw_system(_registry, positions, drawables, _app.getWindow());
}

R_Graphic::App &R_Type::Rtype::getApp()
{
  return _app;
}

engine::registry &R_Type::Rtype::getRegistry()
{
  return _registry;
}

void  R_Type::Rtype::setServerEndpoint(const std::string &ip, unsigned short port)
{
    _serverEndpoint = std::make_unique<engine::net::Endpoint>(
        engine::net::make_endpoint(ip, port));
}

void R_Type::Rtype::waiting_connection()
{
  if (!_connected)
  {
    if (auto pkt_opt = _client->receive(_sender))
    {
      auto [recvHdr, payload] = *pkt_opt;
      if (recvHdr.type == CONNECT_ACK &&
          payload.size() >= sizeof(ConnectAck))
      {
        ConnectAck ack{};
        std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
        _player = ack.playerEntityId;
        _connected = true;
        _registry.spawn_entity();
      }
    }
  }
}

void R_Type::Rtype::handle_collision(engine::registry &reg, size_t i, size_t j)
{
  auto &positions = reg.get_components<component::position>();
  auto &hitboxes = reg.get_components<component::hitbox>();
  auto &kinds = reg.get_components<component::entity_kind>();
  auto &drawables = reg.get_components<component::drawable>();
  auto &animations = reg.get_components<component::animation>();

  auto kindI = (i < kinds.size() && kinds[i]) ? kinds[i].value() : component::entity_kind::unknown;
  auto kindJ = (j < kinds.size() && kinds[j]) ? kinds[j].value() : component::entity_kind::unknown;

  if ((kindI == component::entity_kind::playerProjectile && kindJ == component::entity_kind::enemy) ||
      (kindJ == component::entity_kind::playerProjectile && kindI == component::entity_kind::enemy))
  {
    size_t enemyIdx = (kindI == component::entity_kind::enemy) ? i : j;
    float x = positions[enemyIdx]->x;
    float y = positions[enemyIdx]->y;
    auto explosion = reg.spawn_entity();
    reg.add_component(explosion, component::position{x, y});
    reg.add_component(explosion, component::entity_kind::decor);
    component::animation anim = _playerData->explosionAnimation;
    reg.add_component(explosion, component::lifetime{0.8f});
    reg.add_component(explosion, component::drawable{
      _playerData->texture,
      _playerData->explosionRect,
      layers::Effects});
    reg.add_component(explosion, component::animation{anim});
    return;
  }
}

void R_Type::setAnimation(component::animation &anim, const std::string &clip, bool reverse)
{
  if (anim.currentClip != clip && anim.clips.find(clip) != anim.clips.end())
  {
    anim.currentClip = clip;
    anim.currentFrame = 0;
    anim.timer = 0.f;
    anim.reverse = reverse;
  }
}
