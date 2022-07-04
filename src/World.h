#ifndef WORLD_H
#define WORLD_H



struct Atom
{
    Vec2 pos_cur;
    Vec2 pos_old;
    Vec2 accel;
};



class World
{
public:
    World();
    virtual ~World();

    void Update();

private:
    std::vector< Atom > m_Atoms;
};



#endif // WORLD_H
