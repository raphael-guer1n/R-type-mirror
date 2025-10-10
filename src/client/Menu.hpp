#pragma once
#include <memory>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/events/Events.hpp"

namespace R_Type {
    class Menu {
    public:
        explicit Menu(engine::R_Graphic::App &app);
        ~Menu() = default;

        // Affiche et gère le menu — renvoie true si le jeu doit démarrer
        bool update(const std::vector<engine::R_Events::Event> &events);

        // Dessine le menu (fond + boutons)
        void draw();

    private:
        engine::R_Graphic::App &_app;
        std::shared_ptr<engine::R_Graphic::Texture> _background;
        std::shared_ptr<engine::R_Graphic::Texture> _startButton;
        std::shared_ptr<engine::R_Graphic::Texture> _quitButton;

        bool _startPressed = false;
        bool _quitPressed = false;
    };
}
