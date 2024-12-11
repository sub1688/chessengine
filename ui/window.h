#pragma once
#include <SFML/Graphics.hpp>

namespace BoardWindow {
    inline sf::Texture* pieceTextures[12];

    inline int mouseX = -1;
    inline int mouseY = -1;
    inline int draggingSquare = -1;

    void init();
    void update(sf::RenderWindow& window);
    void loadPieceTextures();
    void destroy();
}