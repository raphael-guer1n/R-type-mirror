/**
 * @file layers.hpp
 * @brief Defines rendering layer constants used to control draw order in the R-Type engine.
 *
 * This header provides a set of named integer constants representing rendering layers.
 * These layers determine the drawing order of graphical elements such as backgrounds,
 * entities, HUD components, and menu interfaces.
 *
 * @note Lower layer values are rendered first (appear behind), while higher values are rendered later (appear in front).
 */

#pragma once

/**
 * @namespace layers
 * @brief Contains integer constants defining the rendering order of various visual elements.
 *
 * The `layers` namespace defines depth-ordering constants for rendering operations.
 * Each element in the game (background, player, enemies, projectiles, HUD, etc.) is assigned a layer index.
 * This system ensures consistent and predictable rendering order across all scenes.
 *
 * @note All values are compile-time constants (`constexpr int`).
 */
namespace layers
{
    /** @brief Background layer — drawn first, behind all other elements. */
    constexpr int Background = 0;

    /** @brief Layer for all projectile entities (e.g., bullets, missiles). */
    constexpr int Projectiles = 5;

    /** @brief Layer for player entities. */
    constexpr int Players = 10;

    /** @brief Layer for enemy entities. */
    constexpr int Enemies = 10;

    /** @brief Layer for particle effects, explosions, or special visual effects. */
    constexpr int Effects = 20;

    /** @name Heads-Up Display (HUD) Layers */
    ///@{
    constexpr int HudBase = 100;   /**< Base HUD layer (background panels, base shapes). */
    constexpr int HudText = 110;   /**< Text elements displayed on the HUD. */
    constexpr int HudIcons = 120;  /**< Icons and indicators rendered above HUD text. */
    ///@}

    /** @name Menu Layers */
    ///@{
    constexpr int MenuBackground = 50; /**< Background of menu screens. */
    constexpr int MenuWidgets = 60;    /**< Interactive menu widgets and buttons. */
    ///@}
}
