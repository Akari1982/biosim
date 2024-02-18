#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>



enum AtomType
{
    COMMON,
    LIGHT,
    LIPID_TRUE,
    LIPID_PRE,
    CELL,
    LIPID,
    FLAGELLIUM,
};


struct Atom
{
    AtomType type;
    float radius;
    float interaction;
    sf::Vector2f pos;
    sf::Vector2f velocity;
    sf::Vector2f force;
    float dir;
    float angular_vel;
    float torque;
    sf::CircleShape ball;
    sf::CircleShape sphere;

    bool garbage;
};



class World
{
public:
    World();
    virtual ~World();

    void Update();
    void Draw(sf::RenderWindow& window);

private:
    void PhysicsUpdate(const float delta);
    void SpawnUpdate(const float delta);
    void RemoveUpdate();

    void AddAtom(const AtomType type, const float radius, const float interaction, const sf::Vector2f &pos, const float dir);

private:
    sf::Clock m_Clock;

    std::vector<Atom> m_Atoms;
};



#endif // WORLD_H
