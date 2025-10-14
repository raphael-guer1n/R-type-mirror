#include <SDL_image.h>
#include <sstream>
#include "Texture.hpp"
#include "Error.hpp"

/**
 * @file Texture.cpp
 * @brief Implémentation de la classe Texture pour la gestion et l'affichage des textures avec SDL2.
 */

/// @brief Constructeur de la classe Texture. Charge une image depuis un fichier, crée une texture SDL et initialise sa position et sa taille.
/// @param window Référence vers la fenêtre SDL utilisée pour la création de la texture.
/// @param filepath Chemin du fichier image à charger.
/// @param pos Position initiale de la texture à l'écran.
/// @param size Taille de la texture (si (0,0), la taille de l'image chargée sera utilisée).
/// @throws R_Graphic::Error si le chargement de l'image ou la création de la texture échoue.

/// @brief Dessine la texture sur la fenêtre spécifiée.
/// @param window Référence vers la fenêtre SDL où dessiner la texture.
/// @param srcrect Rectangle source optionnel pour dessiner seulement une partie de la texture (nullptr pour dessiner toute la texture).

/// @brief Retourne la taille actuelle de la texture.
/// @return Taille de la texture sous forme de intVec2.

/// @brief Définit la position de la texture à l'écran.
/// @param x Nouvelle position horizontale.
/// @param y Nouvelle position verticale.

/// @brief Destructeur de la classe Texture. Libère les ressources associées à la texture SDL.

engine::R_Graphic::Texture::Texture(R_Graphic::Window& window,
    const std::string &filepath, R_Graphic::doubleVec2 pos, R_Graphic::intVec2 size)
: position(pos), _size(size), _texture(nullptr)
{
    std::ostringstream oss;
    SDL_Surface* surface = IMG_Load(filepath.c_str());

    if (!surface) {
        oss << "Texture: Error load img: " << IMG_GetError();
        throw R_Graphic::Error(oss.str());
    }

    _texture = SDL_CreateTextureFromSurface(window.getRenderer(), surface);
    if (!_texture) {
        oss << "Texture: Error creating texture: " << SDL_GetError();
        SDL_FreeSurface(surface);
        throw R_Graphic::Error(oss.str());
    }

    if (_size.x == 0 || _size.y == 0) {
        _size.x = surface->w;
        _size.y = surface->h;
    }
    SDL_FreeSurface(surface);
}

void engine::R_Graphic::Texture::draw(R_Graphic::Window& window, R_Graphic::textureRect* srcrect) {
    SDL_Rect dst = {
        static_cast<int>(position.x),
        static_cast<int>(position.y),
        _size.x, _size.y
    };

    if (!srcrect) {
        SDL_RenderCopy(window.getRenderer(), _texture, nullptr, &dst);
    } else {
        SDL_Rect rect = {
            srcrect->pos.x, srcrect->pos.y,
            srcrect->size.x, srcrect->size.y
        };
        SDL_RenderCopy(window.getRenderer(), _texture, &rect, &dst);
    }
}

void engine::R_Graphic::Texture::changeColors(int r, int g, int b)
{
    SDL_SetTextureColorMod(_texture, r, g, b);
}

engine::R_Graphic::intVec2 engine::R_Graphic::Texture::getSize() const {
    return _size;
}

void engine::R_Graphic::Texture::setPosition(double x, double y) {
    position.x = x;
    position.y = y;
}

engine::R_Graphic::Texture::~Texture() {
    if (_texture)
        SDL_DestroyTexture(_texture);
}
