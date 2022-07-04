#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>



struct Atom
{
    sf::Vector2f pos_cur;
    sf::Vector2f pos_old;
    sf::Vector2f accel;
    float radius;
    sf::CircleShape ball;
};



class World
{
public:
    World();
    virtual ~World();

    void Update();
    void Draw(sf::RenderWindow& window);

private:
    sf::Clock m_Clock;
    std::vector< Atom > m_Atoms;
    sf::CircleShape m_Constrain;
};



#endif // WORLD_H
