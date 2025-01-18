#pragma once
#include <SFML/Graphics.hpp>

#include "../engine/board.h"

namespace BoardWindow {
    inline sf::Font font;
    inline sf::Texture* pieceTextures[12];

    inline uint64_t zobristKey;
    inline float smoothEvalOffset = 0.0F;
    inline int mouseX = -1;
    inline int mouseY = -1;
    inline int draggingSquare = -1;
    inline bool thinking = false;
    inline int displayBoard[64];
    inline Move lastMove = Move(0, 0);

    void init();
    void update(sf::RenderWindow& window);
    void loadPieceTextures();
    void destroy();
    void playBotMove();
}
