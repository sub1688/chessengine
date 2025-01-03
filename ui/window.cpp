#include "window.h"

#include <array>
#include <thread>
#include <SFML/Graphics.hpp>

#include "../engine/board.h"
#include "../engine/movegen.h"
#include "../engine/search.h"

void BoardWindow::playBotMove() {
    thinking = true;
    std::thread([]() {
        Search::startIterativeSearch(5000);
        Board::move(Search::bestMove);
        lastMove = Search::bestMove;
        thinking = false;
    }).detach(); // Detach thread as it's self-contained
}

void BoardWindow::init() {
    font.loadFromFile("arial.ttf");

    auto window = sf::RenderWindow({1000u, 600u}, "Analysis");
    window.setFramerateLimit(100);

    loadPieceTextures();

    while (window.isOpen()) {
        for (auto event = sf::Event(); window.pollEvent(event);) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseMoved) {
                mouseY = event.mouseMove.y;
                mouseX = event.mouseMove.x;
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                int file = mouseX / 75; // Column (0 to 7)
                int rank = 7 - (mouseY / 75); // Row (0 to 7, flipped for top-down drawing)
                int index = rank * 8 + file; // Convert to 0-based index
                if (Board::getPiece(index) == NONE || thinking)
                    continue;
                draggingSquare = index;
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                int file = mouseX / 75; // Column (0 to 7)
                int rank = 7 - (mouseY / 75); // Row (0 to 7, flipped for top-down drawing)
                int index = rank * 8 + file; // Convert to 0-based index

                ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();

                for (int i = 0; i < moves.elements; i++) {
                    Move move = moves.buffer[i];
                    if (move.from == draggingSquare && move.to == index) {
                        if (Board::move(move)) {
                            lastMove = move;
                            for (int i = 0; i < 64; i++) {
                                displayBoard[i] = Board::getPiece(i);
                            }
                            thinking = true;
                            playBotMove();
                            break;
                        }
                    }
                }

                draggingSquare = -1;
            }
        }

        window.clear();
        update(window);
        window.display();
    }
    destroy();
}

void BoardWindow::update(sf::RenderWindow &window) {
    if (!thinking) {
        for (int i = 0; i < 64; i++) {
            displayBoard[i] = Board::getPiece(i);
        }
    }

    window.clear(sf::Color(171, 126, 89));

    // Draw Squares
    for (int i = 0; i < 64; i++) {
        int ix = i % 8;
        int iy = i / 8;
        sf::RectangleShape rect(sf::Vector2f(75, 75));
        rect.setFillColor((i + iy) % 2 == 0 ? sf::Color(240, 217, 181) : sf::Color(181, 136, 99));
        rect.setPosition(sf::Vector2f(ix * 75, iy * 75));
        window.draw(rect);
    }

    if (lastMove.from != lastMove.to) {
        {
            int ix = lastMove.to % 8;
            int iy = 7 - lastMove.to / 8;
            sf::RectangleShape rect(sf::Vector2f(75, 75));
            rect.setFillColor(sf::Color(181, 181, 25, 75));
            rect.setPosition(sf::Vector2f(ix * 75.F, iy * 75.F));
            window.draw(rect);
        } {
            int ix = lastMove.from % 8;
            int iy = 7 - lastMove.from / 8;
            sf::RectangleShape rect(sf::Vector2f(75, 75));
            rect.setFillColor(sf::Color(181, 181, 25, 75));
            rect.setPosition(sf::Vector2f(ix * 75.F, iy * 75.F));
            window.draw(rect);
        }
    }

    // Draw Pieces
    for (int i = 0; i < 64; i++) {
        int piece = displayBoard[i];
        if (piece == NONE || i == draggingSquare)
            continue;
        int ix = i % 8;
        int iy = 7 - (i / 8);
        sf::Sprite sprite;
        sprite.setTexture(*pieceTextures[piece]);
        sprite.setPosition(sf::Vector2f(75.F * ix, 75.F * iy));
        window.draw(sprite);
    }

    if (draggingSquare != -1) {
        ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();

        for (int i = 0; i < moves.elements; i++) {
            Move move = moves.buffer[i];
            if (move.from == draggingSquare && Board::move(move)) {
                Board::undoMove(move);

                int ix = move.to % 8;
                int iy = 7 - move.to / 8;
                sf::RectangleShape rect(sf::Vector2f(75, 75));
                rect.setFillColor(sf::Color(181, 24, 25, 75));
                rect.setPosition(sf::Vector2f(ix * 75.F, iy * 75.F));
                window.draw(rect);
            }
        }

        sf::Sprite sprite;
        sprite.setTexture(*pieceTextures[Board::getPiece(draggingSquare)]);
        sprite.setPosition(sf::Vector2f(mouseX - 37.5F, mouseY - 37.5F));
        window.draw(sprite);
    }

    sf::Text text;
    text.setFillColor(sf::Color::White);
    text.setFont(font);
    text.setString(
        "Depth: " + std::to_string(Search::currentDepth) + " Eval: " + std::to_string(
            static_cast<float>(-Search::currentEval) / 100.F));
    text.setCharacterSize(25);
    text.setPosition(sf::Vector2f(75 * 8 + 5, 5));
    window.draw(text);

}

void BoardWindow::destroy() {
}

void BoardWindow::loadPieceTextures() {
    static sf::Texture textureWP;
    textureWP.loadFromFile("assets/wP.png");
    pieceTextures[0] = &textureWP;
    static sf::Texture textureWN;
    textureWN.loadFromFile("assets/wN.png");
    pieceTextures[1] = &textureWN;
    static sf::Texture textureWB;
    textureWB.loadFromFile("assets/wB.png");
    pieceTextures[2] = &textureWB;
    static sf::Texture textureWQ;
    textureWQ.loadFromFile("assets/wQ.png");
    pieceTextures[3] = &textureWQ;
    static sf::Texture textureWK;
    textureWK.loadFromFile("assets/wK.png");
    pieceTextures[4] = &textureWK;
    static sf::Texture textureWR;
    textureWR.loadFromFile("assets/wR.png");
    pieceTextures[5] = &textureWR;
    static sf::Texture textureBP;
    textureBP.loadFromFile("assets/bP.png");
    pieceTextures[6] = &textureBP;
    static sf::Texture textureBN;
    textureBN.loadFromFile("assets/bN.png");
    pieceTextures[7] = &textureBN;
    static sf::Texture textureBB;
    textureBB.loadFromFile("assets/bB.png");
    pieceTextures[8] = &textureBB;
    static sf::Texture textureBQ;
    textureBQ.loadFromFile("assets/bQ.png");
    pieceTextures[9] = &textureBQ;
    static sf::Texture textureBK;
    textureBK.loadFromFile("assets/bK.png");
    pieceTextures[10] = &textureBK;
    static sf::Texture textureBR;
    textureBR.loadFromFile("assets/bR.png");
    pieceTextures[11] = &textureBR;
}
