#include <SFML/Graphics.hpp>
#include <iostream>
#include <cassert>
#include <vector>

namespace MyGameGraphics
{
    class Board : public sf::Drawable, public sf::Transformable {
    public:
        Board(unsigned int left, 
              unsigned int top, 
              unsigned int length, 
              unsigned width, 
              unsigned int side)
        {
            grid.setPrimitiveType(sf::Lines);
            grid.resize(2 * (width + length + 2));
            
            surface.setPrimitiveType(sf::Quads);
            surface.resize(4);

            surface[0].position = sf::Vector2f(left, top);
            surface[1].position = sf::Vector2f(left + side * width, top);
            surface[2].position = sf::Vector2f(left + side * width, top + side * length);
            surface[3].position = sf::Vector2f(left, top + side * length);

            surface[0].color = sf::Color::Green;
            surface[1].color = sf::Color::Green;
            surface[2].color = sf::Color::Green;
            surface[3].color = sf::Color::Green;

            for(unsigned int i = 0; i <= length; i++)
            {
                sf::Vertex* ln =  &grid[2*i];
                ln[0].position = sf::Vector2f(left, top + i * side);
                ln[1].position = sf::Vector2f(left + side * width, top + i * side);
                ln[0].color = ln[1].color = sf::Color::White;
            }
            for(unsigned int i = 0; i <= width; i++)
            {
                sf::Vertex* ln =  &grid[2*(i + length + 1)];
                ln[0].position = sf::Vector2f(left + i * side, top);
                ln[1].position = sf::Vector2f(left + i * side, top + length * side);
                ln[0].color = ln[1].color = sf::Color::White;
            }
        }
    private:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
        {
            states.transform *= getTransform();
            target.draw(surface);
            target.draw(grid, states);
        }
        sf::VertexArray surface;
        sf::VertexArray grid;
    };
}

int main()
{
    const int radius = 36;
    const int width = 600;
    const int length = 700;
    sf::RenderWindow window(sf::VideoMode(width,length), "Othello", sf::Style::Titlebar | sf::Style::Close);
    MyGameGraphics::Board board(0, 100, 8, 8, 75);
    sf::CircleShape pcWhite(radius), pcBlack(radius);
    pcWhite.setFillColor(sf::Color::White);
    pcWhite.setOutlineThickness(-1.f);
    pcWhite.setPosition(0,100);
    pcWhite.setOutlineColor(sf::Color::Black);
    pcBlack.setFillColor(sf::Color::Black);
    pcBlack.setPosition(0,100);
    int color = 0;
    while(window.isOpen())
    {
        sf::Event event;
    
        while(window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed: { 
                    window.close();
                    break;
                }
                case sf::Event::MouseMoved: {
                    if(color) {
                        int posx = event.mouseMove.x, 
                            posy = event.mouseMove.y;

                        if(posx >  width - radius)
                            posx = width - radius;
                        else if(posx < radius)
                            posx = radius;

                        if(posy > length - radius)
                            posy = length - radius;
                        else if(posy < 100 + radius)
                            posy = 100 + radius;

                        pcBlack.setPosition(posx - radius, posy - radius);
                        pcWhite.setPosition(posx - radius, posy - radius);
                    }
                } break;
                case sf::Event::MouseButtonPressed: {
                    if(event.mouseButton.button == sf::Mouse::Left)
                        color = 1;
                    else if(event.mouseButton.button == sf::Mouse::Right)
                        color = 2;
                    else
                        color = 0;
                } break;
                default: break;
            }
        }
        window.clear(sf::Color(192,192,192));
        window.draw(board);

        if(color == 1) {
            window.draw(pcBlack);
        }
        else if(color == 2){
            window.draw(pcWhite);
        }
        window.display();
    }
    return 0;
}

