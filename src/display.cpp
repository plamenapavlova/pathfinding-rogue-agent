#include"display.hpp"



namespace dspl {

    bool InitDisplay(unsigned int width, unsigned int height, WorldConfig & config) {
        disp->window_size = sf::Vector2u(width, height);
        disp->window = sf::RenderWindow(sf::VideoMode(disp->window_size), "Microworld");
        disp->window.setVerticalSyncEnabled(true);

        if(!disp->font.openFromFile("cmuttr.ttf")) {
            std::cerr << "Missing font file cmuttr.ttf" << std::endl;
            return false;
        } else {
            
        }

        std::vector<unsigned> & v = config.wall_color;
        colors.wall = sf::Color(v[0],v[1],v[2]);
        v = config.open_color;
        colors.open = sf::Color(v[0],v[1],v[2]);

        return true;
    }

    const std::optional<sf::Event> PollEvent() {
        if(disp) {
            return disp->window.pollEvent();
        }
        return {};
    }

    bool IsOpen() {
        if(disp) {
            return disp->window.isOpen();
        } else return false;
    }

    void Close() {
        if(disp) disp->window.close();
    }

    void Clear() {
        disp->window.clear();
    }
    void Display() {
        disp->window.display();
    }

    void DrawMap(const wmap & map, const WorldConfig & config) {
        sf::RectangleShape rect({CELL_SIZE, CELL_SIZE});
        sf::Color color;
        for(size_t i = 0; i < map.size(); i++) {
            for(size_t k = 0; k < map[i].size(); k++) {
                bool goal_present = false;
                std::string cell = map[i][k];
                
                
                if(cell == config.wall) color = colors.wall;
                else color = colors.open;
                rect.setFillColor(color);
                rect.setPosition({ k * CELL_SIZE, i * CELL_SIZE });
                disp->window.draw(rect);

		color = sf::Color::Black;

                if(cell != config.wall && 
                    cell != config.open &&
                    cell != config.agent_start
                    ) {
                    sf::Text text(disp->font);
                    text.setCharacterSize(static_cast<unsigned>(CELL_SIZE));
                    text.setString(cell);
                    text.setPosition({k*CELL_SIZE+10.0f,i*CELL_SIZE-5.f});
                    text.setFillColor(color);
                    disp->window.draw(text);
                }
            }
        }
    }

    void DrawAgent(
        Vec2 loc,
        Vec2 heading,
        std::string character,
        sf::Color color) {

        sf::Text text(disp->font);
        text.setCharacterSize(static_cast<unsigned>(CELL_SIZE));
        text.setString(character);
        text.setPosition({loc.x*CELL_SIZE+8.0f,loc.y*CELL_SIZE});
        text.setFillColor(color);
        //text.setStyle(sf::Text::Bold);
        disp->window.draw(text);

        sf::CircleShape c(4.f, 5);
        c.setFillColor(sf::Color::White);
        sf::Vector2f center;
        if (heading == NORTH) {
            center = sf::Vector2f(
                loc.x * CELL_SIZE + CELL_SIZE / 2,
                loc.y * CELL_SIZE);
        }
        else if (heading == SOUTH) {
            center = sf::Vector2f(
                loc.x * CELL_SIZE + CELL_SIZE / 2,
                loc.y * CELL_SIZE + CELL_SIZE);
        }
        else if (heading == EAST) {
            center = sf::Vector2f(
                loc.x * CELL_SIZE + CELL_SIZE,
                loc.y * CELL_SIZE + CELL_SIZE/2);
        }
        else if (heading == WEST) {
            center = sf::Vector2f(
                loc.x * CELL_SIZE,
                loc.y * CELL_SIZE + CELL_SIZE / 2);
        }
        c.setPosition(center);
        disp->window.draw(c);
    }

    void DrawMsg(std::string msg, float x, float y, int size, sf::Color color) {
        sf::Text text(disp->font);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition({x,y});
        text.setString(msg);
        disp->window.draw(text);
    }
};
