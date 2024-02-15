#include "World.h"

#include <ctime>



sf::Vector2f gravity( 0.0f, 0.0f );

const sf::Vector2f const_pos(400.0f, 350.0f);
const float const_radius = 350.0f;

float spawn = 0;
float spawn_l = 0;
int count = 0;



World::World()
{
    m_Constrain.setRadius( const_radius );
    m_Constrain.setFillColor( sf::Color::Black );
    m_Constrain.setOrigin( const_radius, const_radius );
    m_Constrain.setPosition( const_pos );

    // Create lamp
    Atom atom;
    atom.radius = 20;
    atom.pos_cur = const_pos;
    atom.pos_old = atom.pos_cur;
    atom.accel = sf::Vector2f(0, 0);
    atom.accel_cur = atom.accel;
    atom.fixed = true;
    atom.solid = true;
    atom.ball.setRadius(atom.radius);
    sf::Color color(255, 255, 60);
    atom.ball.setFillColor(color);
    atom.ball.setOrigin(atom.radius, atom.radius);
    atom.ball.setPosition(atom.pos_cur);
    m_Atoms.push_back(atom);

    for (int i = 0; i < 40; ++i)
    {
        Atom atom;
        atom.radius = 5;
        atom.pos_cur = sf::Vector2f(100.0f + i * 10, 250.0f);
        atom.pos_old = sf::Vector2f(100.0f + i * 10, 250.0f);
        atom.accel = sf::Vector2f(0, 0);
        atom.accel_cur = sf::Vector2f(0, 0);
        if( i == 0 || i == 39 ) atom.fixed = true;
        atom.solid = true;

        atom.ball.setRadius(atom.radius);
        sf::Color color(255, 255, 255);
        atom.ball.setFillColor(color);
        atom.ball.setOrigin(atom.radius, atom.radius);
        atom.ball.setPosition(atom.pos_cur);

        m_Atoms.push_back(atom);
/*
        if (i > 0)
        {
            Link link;
            link.id1 = i - 1;
            link.id2 = i;
            link.dist = 10;
            m_Links.push_back(link);
        }
*/
    }
}



World::~World()
{
}



void
World::Update()
{
    const float delta = m_Clock.restart().asSeconds();

    PhysicsUpdate(delta);
    SpawnUpdate(delta);
}



void
World::PhysicsUpdate(const float delta)
{
    const int steps = 16;
    const float sub_delta = delta / steps;

    for(int sub_steps = 0; sub_steps < steps; ++sub_steps)
    {
        // apply gravity
        for(int i = 0; i < m_Atoms.size(); i++)
        {
            m_Atoms[i].accel_cur += gravity;
        }

        // apply constraint
        for(std::vector<Atom>::iterator i = m_Atoms.begin(); i != m_Atoms.end();)
        {
            const sf::Vector2f to_obj = i->pos_cur - const_pos;
            const float dist = sqrt( to_obj.x * to_obj.x + to_obj.y * to_obj.y );
            if(dist > const_radius - i->radius)
            {
                if(i->solid == true)
                {
                    const sf::Vector2f n = to_obj / dist;
                    i->pos_cur = const_pos + n * (const_radius - i->radius);
                    ++i;
                }
                else
                {
                    i = m_Atoms.erase(i);
                }
            }
            else
            {
                ++i;
            }
        }

        // solve links
        for(int i = 0; i < m_Links.size(); i++)
        {
            Atom& obj1 = m_Atoms[m_Links[i].id1];
            Atom& obj2 = m_Atoms[m_Links[i].id2];

            const sf::Vector2f axis = obj1.pos_cur - obj2.pos_cur;
            const float dist = sqrt(axis.x * axis.x + axis.y * axis.y);
            const sf::Vector2f n = axis / dist;
            const float delta = m_Links[i].dist - dist;

            if(obj1.fixed == false)
                obj1.pos_cur += 0.5f * delta * n;
            else
                obj2.pos_cur -= 0.5f * delta * n;

            if(obj2.fixed == false)
                obj2.pos_cur -= 0.5f * delta * n;
            else
                obj1.pos_cur += 0.5f * delta * n;
        }

        // solve collisions
        for(int i = 0; i < m_Atoms.size(); i++)
        {
            if(m_Atoms[i].solid == false) continue;

            for(int j = i + 1; j < m_Atoms.size(); j++)
            {
                if(m_Atoms[j].solid == false) continue;

                const sf::Vector2f coll_axis = m_Atoms[i].pos_cur - m_Atoms[j].pos_cur;
                const float dist = sqrt(coll_axis.x * coll_axis.x + coll_axis.y * coll_axis.y);
                if(dist < m_Atoms[i].radius + m_Atoms[j].radius)
                {
                    const sf::Vector2f n = coll_axis / dist;
                    const float delta = m_Atoms[i].radius + m_Atoms[j].radius - dist;

                    if (m_Atoms[i].fixed == false)
                        m_Atoms[i].pos_cur += 0.5f * delta * n;
                    else
                        m_Atoms[j].pos_cur -= 0.5f * delta * n;

                    if (m_Atoms[j].fixed == false)
                        m_Atoms[j].pos_cur -= 0.5f * delta * n;
                    else
                        m_Atoms[i].pos_cur += 0.5f * delta * n;
                }
            }
        }

        // calculate position
        for(int i = 0; i < m_Atoms.size(); i++)
        {
            if(m_Atoms[i].fixed == true) continue;

            sf::Vector2f velocity = m_Atoms[i].pos_cur - m_Atoms[i].pos_old;
            m_Atoms[i].pos_old = m_Atoms[i].pos_cur;
            m_Atoms[i].pos_cur = m_Atoms[i].pos_cur + velocity + m_Atoms[i].accel_cur * sub_delta * sub_delta;
            m_Atoms[i].accel_cur = m_Atoms[i].accel;
            m_Atoms[i].ball.setPosition(m_Atoms[i].pos_cur);
        }
    }
}



void
World::SpawnUpdate(const float delta)
{
    if(spawn_l >= 0.03f)
    {
        // Create light
        Atom atom;
        atom.radius = 2;
        atom.pos_cur = sf::Vector2f(const_pos);
        atom.pos_old = atom.pos_cur;
        atom.accel = sf::Vector2f(-50.0f + rand() % 100, -50.0f + rand() % 100);
        atom.accel_cur = atom.accel;
        atom.solid = false;
        atom.ball.setRadius(atom.radius);
        sf::Color color(255, 255, 60);
        atom.ball.setFillColor(color);
        atom.ball.setOrigin(atom.radius, atom.radius);
        atom.ball.setPosition(atom.pos_cur);
        m_Atoms.push_back(atom);

        spawn_l = 0;
    }
    else
    {
        spawn_l += delta;
    }

    if (spawn >= 0.5f && count < 50)
    {
        // Create molecule
        Atom atom;
        atom.radius = 10;
        atom.pos_cur = sf::Vector2f(0.0f + rand() % 1200, 0.0f + rand() % 700);
        atom.pos_old = atom.pos_cur;
        atom.accel = sf::Vector2f(-20.0f + rand() % 40, -20.0f + rand() % 40);
        atom.accel_cur = atom.accel;
        atom.solid = true;
        atom.ball.setRadius(atom.radius);
        sf::Color color(0, 255, 0);
        color = sf::Color(0, 200, 0);
        atom.ball.setFillColor(color);
        atom.ball.setOrigin(atom.radius, atom.radius);
        atom.ball.setPosition(atom.pos_cur);
        m_Atoms.push_back(atom);


        spawn = 0;
        count += 1;
    }
    else
    {
        spawn += delta;
    }
}



void
World::Draw( sf::RenderWindow& window )
{
    window.draw( m_Constrain );

    static sf::Vertex line[] =
    {
        sf::Vertex(sf::Vector2f(0, 0), sf::Color::White, sf::Vector2f(0, 0)),
        sf::Vertex(sf::Vector2f(0, 0), sf::Color::White, sf::Vector2f(0, 0))
    };
    for (int i = 0; i < m_Links.size(); i++)
    {
        line[0].position = m_Atoms[m_Links[i].id1].pos_cur;
        line[1].position = m_Atoms[m_Links[i].id2].pos_cur;
        window.draw( line, 2, sf::Lines);
    }

    for( int i = 0; i < m_Atoms.size(); i++ )
    {
        window.draw( m_Atoms[ i ].ball );
    }
}
