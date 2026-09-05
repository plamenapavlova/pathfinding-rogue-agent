#pragma once

#include<algorithm>
#include<iostream>
#include<memory>
#include<SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include"defs.hpp"
#include"vec2.hpp"

namespace dspl {

    struct Display {
        sf::RenderWindow window;
        sf::Vector2u window_size;
        sf::Font font;
    };

    struct Colors {
        sf::Color wall;
        sf::Color open;
        sf::Color exit;
        sf::Color teleporter1;
        sf::Color teleporter2;
        sf::Color button;
        sf::Color door;
    };

    inline Colors colors;


    inline std::unique_ptr<Display> disp = std::make_unique<Display>();

    bool InitDisplay(unsigned width, unsigned height, WorldConfig & config);
    const std::optional<sf::Event> PollEvent();
    bool IsOpen();
    void Close();


    void Clear();
    void Display();

    void DrawMap(const wmap & map, const WorldConfig & config);
    void DrawAgent(
        Vec2 loc, 
        Vec2 heading, 
        std::string character,
        sf::Color color);
    void DrawMsg(std::string msg, float x, float y, int size=15, sf::Color = sf::Color::White);

};