#include "Background.hpp"
#include "common/Layers.hpp"
#include "engine/ecs/Systems.hpp"
#include "Rtype.hpp"
#include <iostream>

R_Type::Background::Background(R_Type::Rtype& rtype)
    : _rtype(rtype), _bg1(rtype.getRegistry().spawn_entity()), _bg2(rtype.getRegistry().spawn_entity())
{
    auto &registry = _rtype.getRegistry();
    auto size = _rtype.getApp().getWindow().getSize();

    registry.add_component(_bg1, component::position{0.0f, 0.0f});
    registry.add_component(_bg1, component::velocity{-100.0f, 0.0f});
    registry.add_component(_bg1, component::entity_kind{component::entity_kind::decor});

    auto tex1 = std::make_shared<R_Graphic::Texture>(
        _rtype.getApp().getWindow(),
        "./Assets/Background/Starfield.png",
        R_Graphic::doubleVec2(0.0, 0.0),
        size
    );
    _currentTexture = tex1;
    registry.emplace_component<component::drawable>(_bg1, tex1,
        R_Graphic::textureRect{0, 0, size.x, size.y}, layers::Background);

    registry.add_component(_bg2, component::position{static_cast<float>(size.x), 0.0f});
    registry.add_component(_bg2, component::velocity{-100.0f, 0.0f});
    registry.add_component(_bg2, component::entity_kind{component::entity_kind::decor});

    auto tex2 = std::make_shared<R_Graphic::Texture>(
        _rtype.getApp().getWindow(),
        "./Assets/Background/Starfield.png",
        R_Graphic::doubleVec2(0.0, 0.0),
        size
    );
    registry.emplace_component<component::drawable>(_bg2, tex2,
        R_Graphic::textureRect{0, 0, size.x, size.y}, layers::Background);
}

void R_Type::Background::changeTheme(int level)
{
    std::string texturePath;

    switch(level)
    {
        case 1: texturePath = "./Assets/Background/Starfield.png"; break;
        case 2: texturePath = "./Assets/Background/l9.png"; break;
        case 3: texturePath = "./Assets/Background/l10.png"; break;
        default: texturePath = "./Assets/Background/Starfield.png"; break;
    }

    auto newTex = std::make_shared<engine::R_Graphic::Texture>(
        _rtype.getApp().getWindow(),
        texturePath,
        engine::R_Graphic::doubleVec2(0.0, 0.0),
        _rtype.getApp().getWindow().getSize()
    );

    auto &registry = _rtype.getRegistry();
    auto &drawables = registry.get_components<component::drawable>();

    if (_bg1 < drawables.size() && drawables[_bg1])
        drawables[_bg1]->texture = newTex;

    if (_bg2 < drawables.size() && drawables[_bg2])
        drawables[_bg2]->texture = newTex;

    _currentTexture = newTex;
    _nextTexture.reset();     
    _isTransitioning = false; 
}


void R_Type::Background::update(float deltaTime)
{
    if (!_isTransitioning)
        return;

    auto &registry = _rtype.getRegistry();
    auto &drawables = registry.get_components<component::drawable>();

    _fadeAlpha -= 150.f * deltaTime;
    if (_fadeAlpha < 0.f)
        _fadeAlpha = 0.f;

    Uint8 currentAlpha = (Uint8)_fadeAlpha;
    Uint8 nextAlpha = (Uint8)(255 - _fadeAlpha);

    if (_bg1 < drawables.size() && drawables[_bg1] && drawables[_bg1]->texture)
    SDL_SetTextureAlphaMod(drawables[_bg1]->texture->getSDLTexture(), currentAlpha);

    if (_bg2 < drawables.size() && drawables[_bg2] && drawables[_bg2]->texture)
        SDL_SetTextureAlphaMod(drawables[_bg2]->texture->getSDLTexture(), currentAlpha);

    if (_nextTexture)
        SDL_SetTextureAlphaMod(_nextTexture->getSDLTexture(), nextAlpha);

    if (_fadeAlpha == 0.f)
    {
        _isTransitioning = false;
        _fadeAlpha = 255.f;

        if (_bg1 < drawables.size() && drawables[_bg1])
            drawables[_bg1]->texture = _nextTexture;

        if (_bg2 < drawables.size() && drawables[_bg2])
            drawables[_bg2]->texture = _nextTexture;
    }
}
