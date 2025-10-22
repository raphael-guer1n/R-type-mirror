#include "engine/Engine.hpp"
#include <iostream>
#include <cmath>
#include <unordered_map>

using namespace engine::R_Graphic;
using namespace engine;

// Simple color component for rendering (not in engine yet)
struct Color {
    uint8_t r, g, b, a;
};

// Tag components to identify entity types
struct Player {};
struct Platform {};

// Input system for player control
void input_system(registry &reg,
                 sparse_array<component::velocity> &velocities,
                 sparse_array<component::controllable> &controllables,
                 sparse_array<Player> &playerTags,
                 const Uint8* keystate,
                 bool onGround,
                 float moveSpeed,
                 float jumpForce)
{
    for (auto &&[i, vel, ctrl, player] : indexed_zipper(velocities, controllables, playerTags)) {
        // Horizontal movement
        vel.vx = 0.0f;
        if (keystate[SDL_SCANCODE_LEFT]) {
            vel.vx = -moveSpeed;
        }
        if (keystate[SDL_SCANCODE_RIGHT]) {
            vel.vx = moveSpeed;
        }
        
        // Jump
        if (keystate[SDL_SCANCODE_SPACE] && onGround) {
            vel.vy = jumpForce;
            std::cout << "Jump!\n";
        }
    }
}

// Simple gravity system using engine components
void apply_gravity_system(sparse_array<component::velocity> &velocities,
                         sparse_array<component::gravity> &gravities)
{
    for (auto &&[i, vel, grav] : indexed_zipper(velocities, gravities)) {
        vel.vy += grav.ay; // Apply gravity acceleration
    }
}

// Screen wrapping system
void screen_wrap_system(sparse_array<component::position> &positions,
                       sparse_array<Player> &playerTags,
                       float screenWidth)
{
    for (auto &&[i, pos, player] : indexed_zipper(positions, playerTags)) {
        if (pos.x < -40.0f) pos.x = screenWidth;
        if (pos.x > screenWidth) pos.x = -40.0f;
    }
}

// Rendering system
void render_system(SDL_Renderer* sdlRenderer,
                  sparse_array<component::position> &positions,
                  sparse_array<component::hitbox> &hitboxes,
                  sparse_array<Color> &colors)
{
    for (auto &&[i, pos, hb, color] : indexed_zipper(positions, hitboxes, colors)) {
        SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = {
            static_cast<int>(pos.x),
            static_cast<int>(pos.y),
            static_cast<int>(hb.width),
            static_cast<int>(hb.height)
        };
        SDL_RenderFillRect(sdlRenderer, &rect);
    }
}

int main() {
    try {
        // Create window and renderer
        Window window("Doodle Jump - Proof of Concept", 400, 600);
        Renderer renderer(window);
        SDL_Renderer* sdlRenderer = window.getRenderer();
        
        // Create ECS registry
        registry reg;
        
        // Register components (using engine's existing components!)
        reg.register_component<component::position>();
        reg.register_component<component::velocity>();
        reg.register_component<component::gravity>();
        reg.register_component<component::hitbox>();
        reg.register_component<component::controllable>();
        reg.register_component<Color>();
        reg.register_component<Player>();
        reg.register_component<Platform>();
        
        // Physics constants
        const float JUMP_FORCE = -12.0f;
        const float MOVE_SPEED = 5.0f;
        const float GROUND_Y = 500.0f;
        const float deltaTime = 1.0f; // Simple fixed timestep
        
        // Create player entity
        entity_t player = reg.spawn_entity();
        reg.add_component(player, component::position{180.0f, 300.0f});
        reg.add_component(player, component::velocity{0.0f, 0.0f});
        reg.add_component(player, component::gravity{0.5f}); // gravity acceleration
        reg.add_component(player, component::hitbox{40.0f, 40.0f});
        reg.add_component(player, component::controllable{});
        reg.add_component(player, Color{0, 255, 0, 255}); // Green
        reg.add_component(player, Player{});
        
        // Create platforms
        // Ground platform
        entity_t ground = reg.spawn_entity();
        reg.add_component(ground, component::position{50.0f, GROUND_Y});
        reg.add_component(ground, component::hitbox{300.0f, 20.0f});
        reg.add_component(ground, Color{100, 100, 255, 255}); // Blue
        reg.add_component(ground, Platform{});
        
        // Mid-air platform
        entity_t platform1 = reg.spawn_entity();
        reg.add_component(platform1, component::position{150.0f, 350.0f});
        reg.add_component(platform1, component::hitbox{100.0f, 20.0f});
        reg.add_component(platform1, Color{100, 100, 255, 255});
        reg.add_component(platform1, Platform{});
        
        // Higher platform
        entity_t platform2 = reg.spawn_entity();
        reg.add_component(platform2, component::position{80.0f, 200.0f});
        reg.add_component(platform2, component::hitbox{120.0f, 20.0f});
        reg.add_component(platform2, Color{100, 100, 255, 255});
        reg.add_component(platform2, Platform{});
        
        std::cout << "Doodle Jump started!\n";
        std::cout << "Controls: LEFT/RIGHT arrows to move, SPACE to jump\n";
        std::cout << "Close window or press ESC to quit\n";
        
        bool onGround = false;
        
        // Game loop
        bool running = true;
        while (running) {
            // Handle events
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
            
            // Get keyboard state for input
            const Uint8* keystate = SDL_GetKeyboardState(NULL);
            
            // Get component arrays
            auto& positions = reg.get_components<component::position>();
            auto& velocities = reg.get_components<component::velocity>();
            auto& gravities = reg.get_components<component::gravity>();
            auto& hitboxes = reg.get_components<component::hitbox>();
            auto& controllables = reg.get_components<component::controllable>();
            auto& colors = reg.get_components<Color>();
            auto& playerTags = reg.get_components<Player>();
            auto& platformTags = reg.get_components<Platform>();
            
            // Run game systems in order (using engine systems where possible!)
            input_system(reg, velocities, controllables, playerTags, keystate, onGround, MOVE_SPEED, JUMP_FORCE);
            apply_gravity_system(velocities, gravities); // Simple wrapper
            position_system(reg, positions, velocities, deltaTime); // ✅ ENGINE SYSTEM!
            screen_wrap_system(positions, playerTags, 400.0f);
            
            // Use engine's hitbox_system for collision detection! ✅ ENGINE SYSTEM!
            onGround = false;
            
            // Store previous Y position before collision to check if landing from above
            std::unordered_map<size_t, float> prevPlayerY;
            for (auto &&[i, pos, player] : indexed_zipper(positions, playerTags)) {
                prevPlayerY[i] = pos.y - velocities[i].value().vy * deltaTime;
            }
            
            hitbox_system(reg, positions, hitboxes, [&](size_t entityA, size_t entityB) {
                // Check if one is player and other is platform
                bool aIsPlayer = entityA < playerTags.size() && playerTags[entityA].has_value();
                bool bIsPlayer = entityB < playerTags.size() && playerTags[entityB].has_value();
                bool aIsPlatform = entityA < platformTags.size() && platformTags[entityA].has_value();
                bool bIsPlatform = entityB < platformTags.size() && platformTags[entityB].has_value();
                
                size_t playerIdx = aIsPlayer ? entityA : (bIsPlayer ? entityB : -1);
                size_t platformIdx = aIsPlatform ? entityA : (bIsPlatform ? entityB : -1);
                
                if (playerIdx != (size_t)-1 && platformIdx != (size_t)-1 && 
                    velocities[playerIdx] && positions[playerIdx] && hitboxes[playerIdx] && 
                    positions[platformIdx] && hitboxes[platformIdx]) {
                    
                    auto& playerVel = velocities[playerIdx].value();
                    auto& playerPos = positions[playerIdx].value();
                    auto& playerHb = hitboxes[playerIdx].value();
                    auto& platPos = positions[platformIdx].value();
                    auto& platHb = hitboxes[platformIdx].value();
                    
                    // Calculate where the player's bottom was before this frame
                    float prevBottom = prevPlayerY[playerIdx] + playerHb.height;
                    float currentBottom = playerPos.y + playerHb.height;
                    float platformTop = platPos.y;
                    
                    // Only land if:
                    // 1. Player is falling (vy > 0)
                    // 2. Player's previous bottom was above or at platform top
                    // 3. Player's current bottom has penetrated the platform
                    if (playerVel.vy > 0 && 
                        prevBottom <= platformTop + 5.0f &&  // Was above platform (with small tolerance)
                        currentBottom > platformTop) {        // Now overlapping
                        
                        // Snap player to top of platform
                        playerPos.y = platPos.y - playerHb.height;
                        playerVel.vy = 0.0f;
                        onGround = true;
                    }
                }
            });
            
            // Render
            renderer.clear();
            render_system(sdlRenderer, positions, hitboxes, colors);
            renderer.display();
            
            SDL_Delay(16); // Cap at ~60 FPS
        }
        
        std::cout << "Doodle Jump closed.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
