#include "BoardWindow.h"
#include <SFML/Graphics.hpp>

void BoardWindow::init() {
    auto window = sf::RenderWindow({750u, 600u}, "Analysis");
    window.setFramerateLimit(100);


    while (window.isOpen())
    {
        for (auto event = sf::Event(); window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();
        update(window);
        window.display();
    }
}

void BoardWindow::update(sf::RenderWindow& window) {
    window.clear(sf::Color(50, 117, 89));

    for (int i = 0; i < 64; i++) {
        int ix = i % 8;
        int iy = i / 8;
        sf::RectangleShape rect(sf::Vector2f(75, 75));
        rect.setFillColor((i + iy) % 2 == 0 ? sf::Color(135, 168, 154) : sf::Color(101, 123, 120));
        rect.setPosition(sf::Vector2f(ix * 75, iy * 75));
        window.draw(rect);
    }
}

