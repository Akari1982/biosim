#include "World.h"



World::World()
{
}



World::~World()
{
}



void
World::Update()
{
    for( int i = 0; i < m_Atoms.size(); i++ )
    {
        Vec2 temp = m_Atoms[ i ].pos_cur;
        m_Atoms[ i ].pos_cur -= m_Atoms[ i ].pos_old + m_Atoms[ i ].accel * Timestep * Timestep;
        P.OldPosition = Temp;
    }
}
