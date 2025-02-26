#include "window.h"

#include <array>
#include <iomanip>
#include <thread>
#include <SFML/Graphics.hpp>

#include "../engine/board.h"
#include "../engine/movegen.h"
#include "../engine/search.h"
#include "../engine/transpositiontable.h"
#include "../engine/zobrist.h"
#include "../engine/oldsearch.h"

void BoardWindow::playBotMove() {
    thinking = true;
    std::thread([]() {
        if ((board->whiteToMove && whiteIsNew) || (!board->whiteToMove && !whiteIsNew)) {
            Search::startIterativeSearch(*board, 250);
            board->move(Search::bestMove);
            thinking = false;
        } else {
            OldSearch::startIterativeSearch(*board, 250);
            board->move(OldSearch::bestMove);
            thinking = false;
        }
    }).detach(); // Detach thread as it's self-contained
}

void BoardWindow::init(Board *board) {
    BoardWindow::board = board;
    font.loadFromFile("arial.ttf");

    auto window = sf::RenderWindow({1200u, 600u}, "Analysis", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(100);

    loadPieceTextures();

    while (window.isOpen()) {
        for (auto event = sf::Event(); window.pollEvent(event);) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && board->moveNumber > 0) {
                if (event.key.code == sf::Keyboard::Left) {
                    Search::searchCancelled = false;
                    thinking = false;
                }
            }

            if (event.type == sf::Event::MouseMoved) {
                mouseY = event.mouseMove.y;
                mouseX = event.mouseMove.x;
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                int file = mouseX / 75; // Column (0 to 7)
                int rank = 7 - (mouseY / 75); // Row (0 to 7, flipped for top-down drawing)
                int index = rank * 8 + file; // Convert to 0-based index
                if (displayBoard[index] == NONE)
                    continue;
                draggingSquare = index;
                Search::searchCancelled = true;
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                int file = mouseX / 75; // Column (0 to 7)
                int rank = 7 - (mouseY / 75); // Row (0 to 7, flipped for top-down drawing)
                int index = rank * 8 + file; // Convert to 0-based index

                ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard(*board);

                for (int i = 0; i < moves.elements; i++) {
                    Move move = moves.buffer[i];
                    if (move.from == draggingSquare && move.to == index) {
                        if (board->move(move)) {
                            lastMove[board->moveNumber] = move;
                            for (int i = 0; i < 64; i++) {
                                displayBoard[i] = board->getPiece(i);
                            }
                            thinking = false;
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
    if (!thinking && board != nullptr) {
        for (int i = 0; i < 64; i++) {
            displayBoard[i] = board->getPiece(i);
        }

        if (!Movegen::inCheckmate(*board) && !board->isDrawn()) {
            zobristKey = Zobrist::calculateZobristKey(*board);

            // thinking = true;
            // std::thread([]() {
                // Search::startIterativeSearch(*board, 100000);
            // }).detach();
            playBotMove();
        }else {
            if (board->isDrawn()) {
                drawn++;
            }else {
                if ((board->whiteToMove && whiteIsNew) || (!board->whiteToMove && !whiteIsNew)) {
                    oldWon++;
                }else {
                    newWon++;
                }
            }
            whiteIsNew = !whiteIsNew;;
            board->setStartingPosition();
            Search::transpositionTable.clear();
            OldSearch::transpositionTable.clear();
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

    if (lastMove[board->moveNumber].from != lastMove[board->moveNumber].to) {
        {
            int ix = lastMove[board->moveNumber].to % 8;
            int iy = 7 - lastMove[board->moveNumber].to / 8;
            sf::RectangleShape rect(sf::Vector2f(75, 75));
            rect.setFillColor(sf::Color(181, 181, 25, 75));
            rect.setPosition(sf::Vector2f(ix * 75.F, iy * 75.F));
            window.draw(rect);
        } {
            int ix = lastMove[board->moveNumber].from % 8;
            int iy = 7 - lastMove[board->moveNumber].from / 8;
            sf::RectangleShape rect(sf::Vector2f(75, 75));
            rect.setFillColor(sf::Color(181, 181, 25, 75));
            rect.setPosition(sf::Vector2f(ix * 75.F, iy * 75.F));
            window.draw(rect);
        }
    }

    if (Search::bestMove.from != Search::bestMove.to) {
        {
            int ix = Search::bestMove.to % 8;
            int iy = 7 - Search::bestMove.to / 8;
            sf::RectangleShape rect(sf::Vector2f(75, 75));
            rect.setFillColor(sf::Color(25, 181, 25, 75));
            rect.setPosition(sf::Vector2f(ix * 75.F, iy * 75.F));
            window.draw(rect);
        } {
            int ix = Search::bestMove.from % 8;
            int iy = 7 - Search::bestMove.from / 8;
            sf::RectangleShape rect(sf::Vector2f(75, 75));
            rect.setFillColor(sf::Color(25, 181, 25, 75));
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
        ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard(*board);

        for (int i = 0; i < moves.elements; i++) {
            Move move = moves.buffer[i];
            if (move.from == draggingSquare && board->move(move)) {
                board->undoMove(move);

                int ix = move.to % 8;
                int iy = 7 - move.to / 8;
                sf::RectangleShape rect(sf::Vector2f(75, 75));
                rect.setFillColor(sf::Color(181, 24, 25, 75));
                rect.setPosition(sf::Vector2f(ix * 75.F, iy * 75.F));
                window.draw(rect);
            }
        }

        sf::Sprite sprite;
        sprite.setTexture(*pieceTextures[displayBoard[draggingSquare]]);
        sprite.setPosition(sf::Vector2f(mouseX - 37.5F, mouseY - 37.5F));
        window.draw(sprite);
    }

    sf::Text text;
    text.setFillColor(sf::Color::White);
    text.setFont(font);

    std::ostringstream ss1;
    ss1 << std::fixed << std::setprecision(2);
    ss1 << static_cast<float>((Search::lastSearchTurnIsWhite ? 1 : -1) * Search::currentEval) / 100.F;
    std::string str = ss1.str();

    // Check if it's a mate score
    if (abs((Search::lastSearchTurnIsWhite ? 1 : -1) * Search::currentEval) > MATE_THRESHOLD) {
        str = "#" + std::to_string(static_cast<int>((Search::POSITIVE_INFINITY - abs(
                                        static_cast<float>(
                                            (Search::lastSearchTurnIsWhite ? 1 : -1) * Search::currentEval)))
                                   / 2 + 1));
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);

    ss << "Depth: " << std::to_string(Search::currentDepth)
            << "\nEvaluation: " << str
            << (Search::searchCancelled ? " Cancelled" : "")
            << "\nTransposition Table Usage: "
            << (double) Search::transpositionTable.tableEntries / (double) TranspositionTable::TRANSPOSITION_TABLE_SIZE
            *
            (sizeof(TranspositionEntry) * TranspositionTable::TRANSPOSITION_TABLE_SIZE / (double) 1000000)
            << "/"
            << (sizeof(TranspositionEntry) * TranspositionTable::TRANSPOSITION_TABLE_SIZE / (double) 1000000)
            << "MB"
            << "\nTransposition Search Cutoffs: " << std::to_string(Search::transpositionTable.cutoffs)
            << "\nEndgame Bias: " << Search::getEndGameBias(*board) << "\nNew Search Won: " << newWon << "\nOld Search Won: " << oldWon << "\nDrawn: " << drawn << "\n";

    ss << "Time to depth:\n";
    for (int i = 2; i < Search::currentDepth; i++)
    {
        ss << std::to_string(i) << ": " << std::to_string(Search::times[i]) << "ms\n";
    }

    text.setString(ss.str());
    text.setCharacterSize(20);
    text.setPosition(sf::Vector2f(75 * 8 + 35, 5));
    window.draw(text);

    sf::RectangleShape evalBackground(sf::Vector2f(30, 600));
    evalBackground.setFillColor(sf::Color(45, 45, 45, 255));
    evalBackground.setPosition(sf::Vector2f(75 * 8, 0));
    window.draw(evalBackground);

    //y=\frac{300}{-\left(\frac{x}{5}+1\right)}+300

    float trueEval = static_cast<float>((Search::lastSearchTurnIsWhite ? 1 : -1) * Search::currentEval) / 100.F;
    float evalOffset = trueEval >= 0
                           ? 300.F + 300.F / -(trueEval / 5.F + 1.F)
                           : -(300 + 300.F / -(-trueEval / 5.F + 1.F));
    smoothEvalOffset += (evalOffset - smoothEvalOffset) / 24.F;
    float evalHeight = 300.F + smoothEvalOffset;
    sf::RectangleShape evalWhite(sf::Vector2f(30, evalHeight));
    evalBackground.setFillColor(sf::Color(255, 255, 255, 255));
    evalBackground.setPosition(sf::Vector2f(75 * 8, 600 - evalHeight));
    window.draw(evalBackground);
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
