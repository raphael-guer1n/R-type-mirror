#pragma once
#include <string>
#include <chrono>
#include <memory>
#include <functional>
#include "Window.hpp"
#include "Renderer.hpp"

/**
 * @brief Déclaration de la classe App pour gérer la boucle principale d'une application graphique.
 * 
 * Cette classe encapsule une fenêtre (Window) et un renderer (Renderer).
 * Elle fournit une méthode run pour exécuter la boucle principale, prenant en paramètres
 * des fonctions de mise à jour et de dessin du jeu.
 */
namespace R_Graphic
{
    class App {
        public:
            App(const std::string name, const int width, const int height);
            ~App() = default;
            void run(std::function<void(float)> gameUpdate,
                std::function<void()> gameDraw);
            Window& getWindow();
        private:
            std::unique_ptr<Renderer> _renderer;
            Window _window;
    };
}
