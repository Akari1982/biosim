#include "World.h"

#include <ctime>



sf::Vector2f gravity(0.0f, 1000.0f);

const sf::Vector2f const_pos(300.0f, 300.0f);
const float const_radius = 200.0f;

float spawn = 0;
int count = 0;
sf::Color color(255, 255, 255);
bool r = false;
bool g = false;
bool b = false;


World::World()
{
    m_Constrain.setRadius(const_radius);
    m_Constrain.setFillColor(sf::Color::Black);
    m_Constrain.setOrigin(const_radius, const_radius);
    m_Constrain.setPosition(const_pos);
}



World::~World()
{
}



void
World::Update()
{
    const float delta = m_Clock.restart().asSeconds();

    const float sub_delta = delta / 8;
    for (int sub_steps = 0; sub_steps < 8; ++sub_steps)
    {
        // apply gravity
        for (int i = 0; i < m_Atoms.size(); i++)
        {
            m_Atoms[i].accel += gravity;
        }

        // apply constraint
        for (int i = 0; i < m_Atoms.size(); i++)
        {
            const sf::Vector2f to_obj = m_Atoms[i].pos_cur - const_pos;
            const float dist = sqrt(to_obj.x * to_obj.x + to_obj.y * to_obj.y);
            if (dist > const_radius - m_Atoms[i].radius)
            {
                const sf::Vector2f n = to_obj / dist;
                m_Atoms[i].pos_cur = const_pos + n * (const_radius - m_Atoms[i].radius);
            }
        }

        // solve collisions
        for (int i = 0; i < m_Atoms.size(); i++)
        {
            for (int j = i + 1; j < m_Atoms.size(); j++)
            {
                const sf::Vector2f coll_axis = m_Atoms[i].pos_cur - m_Atoms[j].pos_cur;
                const float dist = sqrt(coll_axis.x * coll_axis.x + coll_axis.y * coll_axis.y);
                if (dist < m_Atoms[i].radius + m_Atoms[j].radius)
                {
                    const sf::Vector2f n = coll_axis / dist;
                    const float delta = m_Atoms[i].radius + m_Atoms[j].radius - dist;
                    m_Atoms[i].pos_cur += 0.5f * delta * n;
                    m_Atoms[j].pos_cur -= 0.5f * delta * n;
                }
            }
        }

        // calculate position
        for (int i = 0; i < m_Atoms.size(); i++)
        {
            sf::Vector2f velocity = m_Atoms[i].pos_cur - m_Atoms[i].pos_old;
            m_Atoms[i].pos_old = m_Atoms[i].pos_cur;

            m_Atoms[i].pos_cur = m_Atoms[i].pos_cur + velocity + m_Atoms[i].accel * sub_delta * sub_delta;
            m_Atoms[i].accel = sf::Vector2f(0, 0);

            m_Atoms[i].ball.setPosition(m_Atoms[i].pos_cur);
        }
    }


    if (spawn >= 0.1f && count < 200)
    {
        Atom atom;
        atom.radius = 5 + rand() % 10;
        atom.pos_cur = sf::Vector2f(450.0f, 250.0f);
        atom.pos_old = sf::Vector2f(450.0f, 250.0f);
        atom.accel = sf::Vector2f(0, 0);

        atom.ball.setRadius(atom.radius);
        atom.ball.setFillColor(color);
        color.r = ( r ) ? color.r + rand() % 20 : color.r - rand() % 20;
        if (color.r < 20) r = true;
        if (color.r > 200) r = false;
        color.g = ( g ) ? color.g + rand() % 20 : color.g - rand() % 20;
        if (color.g < 20) g = true;
        if (color.g > 200) g = false;
        color.b = ( b ) ? color.b + rand() % 20 : color.b - rand() % 20;
        if (color.b < 20) b = true;
        if (color.b > 200) b = false;
        atom.ball.setOrigin(atom.radius, atom.radius);
        atom.ball.setPosition(atom.pos_cur);

        m_Atoms.push_back(atom);
        spawn = 0;
        ++count;
    }
    else
    {
        spawn += delta;
    }
}



void
World::Draw(sf::RenderWindow& window)
{
    window.draw(m_Constrain);

    for (int i = 0; i < m_Atoms.size(); i++)
    {
        window.draw(m_Atoms[i].ball);
    }
}
