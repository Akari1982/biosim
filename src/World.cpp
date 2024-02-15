#include "World.h"

#include <ctime>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float g_scale = 1.0f;

static float g_viscosity = 0.998f;
static float g_ang_viscosity = 0.977f;

static float g_div_angle = 0.377f;



float
Vector2Length(const sf::Vector2f &vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}



sf::Vector2f
VectorFromAngle(float angle)
{
    return sf::Vector2f(std::cos(angle), std::sin(angle));
}



float
AngleFromVector(const sf::Vector2f v)
{
    return std::atan2(v.y, v.x);
}



float
DotProduct(const sf::Vector2f& v1, const sf::Vector2f& v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}



sf::Vector2f
RotateVector(const sf::Vector2f& v, float alpha)
{
    float cs = std::cos(alpha);
    float sn = std::sin(alpha);

    sf::Vector2f rot_vec;
    rot_vec.x = v.x * cs - v.y * sn;
    rot_vec.y = v.x * sn + v.y * cs;
    return rot_vec;
}



float
MiddleAngle(float theta1, float theta2)
{
    // Make sure theta1 and theta2 are in the range of [0, 2*PI]
    theta1 = fmod(theta1, 2 * M_PI);
    if (theta1 < 0)
        theta1 += 2 * M_PI;

    theta2 = fmod(theta2, 2 * M_PI);
    if (theta2 < 0)
        theta2 += 2 * M_PI;

    // Compute difference
    double diff = theta2 - theta1;
    if (diff < -M_PI)
        diff += 2 * M_PI;
    else if (diff > M_PI)
        diff -= 2 * M_PI;

    // Compute middle angle
    double middle = theta1 + diff / 2.0;

    // Make sure the middle angle is in the range of [0, 2*PI]
    middle = fmod(middle, 2 * M_PI);
    if (middle < 0)
        middle += 2 * M_PI;

    return middle;
}



void
ÑalculateForceAndTorque_LIPID(const Atom& atom1, const Atom& atom2, const sf::Vector2f& coll_axis, sf::Vector2f& force, float& torque)
{
    float teta = atom2.dir - atom1.dir;
    while (teta > M_PI) teta -= 2 * M_PI;
    while (teta < -M_PI) teta += 2 * M_PI;
    float phi = AngleFromVector(coll_axis) - atom1.dir;
    while (phi > M_PI) phi -= 2 * M_PI;
    while (phi < -M_PI) phi += 2 * M_PI;
    float rot_factor = std::sin(2 * phi - teta + M_PI);
    force = RotateVector(coll_axis, rot_factor) * (10.0f * rot_factor * rot_factor + 1.0f);

    //Something like (phi>0)
    float midAngle = MiddleAngle(atom1.dir, atom2.dir);
    bool isLeft = DotProduct(coll_axis, RotateVector(VectorFromAngle(midAngle), -M_PI / 2.0)) > 0;
    float leftTargetOffset = (-teta - g_div_angle) / 2.0;
    float righTargetOffset = (-teta + g_div_angle) / 2.0;
    while (leftTargetOffset > M_PI) leftTargetOffset -= 2 * M_PI;
    while (leftTargetOffset < -M_PI) leftTargetOffset += 2 * M_PI;
    while (righTargetOffset > M_PI) righTargetOffset -= 2 * M_PI;
    while (righTargetOffset < -M_PI) righTargetOffset += 2 * M_PI;
    float dirFactor = ((isLeft && (leftTargetOffset < 0)) || (!isLeft && (righTargetOffset < 0))) ? 1.0f : -1.0f;
    torque = dirFactor * 100;

    // torque influence as well diminish with distance
    //atom1.torque /= rNorm;
}



World::World()
{
    for (int i = 0; i < 500; ++i)
    {
        sf::Vector2f pos = sf::Vector2f(10.0f + rand() % 1260, 10.0f + rand() % 700);
        AddAtom(AtomType::LIPID, 5.0f, 10.0f, pos, rand() % 10, sf::Color::Red);
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
    RemoveUpdate();
}



void
World::PhysicsUpdate(const float delta)
{
    const int steps = 16;
    const float sub_delta = delta / steps;

    std::random_device rd;
    std::mt19937 gen(rd());

    for (int sub_steps = 0; sub_steps < steps; ++sub_steps)
    {
        // solve interactions
        for (int i = 0; i < m_Atoms.size(); ++i)
        {
            for (int j = i + 1; j < m_Atoms.size(); ++j)
            {
                const sf::Vector2f coll_axis = m_Atoms[i].pos - m_Atoms[j].pos;
                const float dist = Vector2Length(coll_axis);
                if (dist < m_Atoms[i].interaction + m_Atoms[j].interaction)
                {
                    const sf::Vector2f n = coll_axis / dist;

                    if (m_Atoms[i].type == AtomType::LIPID && m_Atoms[j].type == AtomType::LIPID)
                    {
                        const float dist_inf = 1 - dist / (m_Atoms[i].interaction + m_Atoms[j].interaction);
                        sf::Vector2f force;
                        float torque;
                        ÑalculateForceAndTorque_LIPID(m_Atoms[i], m_Atoms[j], -coll_axis, force, torque);
                        m_Atoms[i].force += force * dist_inf;
                        m_Atoms[i].torque += torque * dist_inf;
                        ÑalculateForceAndTorque_LIPID(m_Atoms[j], m_Atoms[i], coll_axis, force, torque);
                        m_Atoms[j].force += force * dist_inf;
                        m_Atoms[j].torque += torque * dist_inf;
                    }
                    else
                    {
                        const float dist_inf = 1 - dist / (m_Atoms[i].interaction + m_Atoms[j].interaction);
                        m_Atoms[i].force += n * dist_inf;
                        m_Atoms[j].force -= n * dist_inf;
                    }

                    // atom radius interaction
                    if (dist < m_Atoms[i].radius + m_Atoms[j].radius)
                    {
                        const float dist_inf = 1 - dist / (m_Atoms[i].radius + m_Atoms[j].radius);
                        m_Atoms[i].force += n * 1000.0f * dist_inf;
                        m_Atoms[j].force -= n * 1000.0f * dist_inf;
                    }
                }
            }
        }

        // add brownian movement
        for (int i = 0; i < m_Atoms.size(); ++i)
        {
            std::uniform_real_distribution<> disBrownian(-10, 10);
            m_Atoms[i].force += sf::Vector2f(disBrownian(gen), disBrownian(gen));
        }

        // calculate position
        for (int i = 0; i < m_Atoms.size(); ++i)
        {
            if (Vector2Length(m_Atoms[i].velocity) <= 1.0)
            {
                m_Atoms[i].force += 0.01f * m_Atoms[i].velocity / sub_delta;
            }

            m_Atoms[i].velocity += m_Atoms[i].force * sub_delta;
            m_Atoms[i].angular_vel += m_Atoms[i].torque * sub_delta;

            // cap velocity
            float max_vel = 10.0f;
            sf::Vector2f vel = m_Atoms[i].velocity;
            const float len_vel = Vector2Length(vel);
            if (len_vel > max_vel)
            {
                if (len_vel != 0) vel /= len_vel;
                m_Atoms[i].velocity = vel *= max_vel;
            }

            // cap angular velocity
            float max_ang_vel = 10.0;
            if (m_Atoms[i].angular_vel > max_ang_vel) m_Atoms[i].angular_vel = max_ang_vel;
            if (m_Atoms[i].angular_vel < -max_ang_vel) m_Atoms[i].angular_vel = -max_ang_vel;

            m_Atoms[i].velocity *= g_viscosity;
            m_Atoms[i].angular_vel *= g_ang_viscosity;

            m_Atoms[i].pos += m_Atoms[i].velocity * sub_delta;
            m_Atoms[i].dir += m_Atoms[i].angular_vel * sub_delta;

            m_Atoms[i].ball.setPosition(m_Atoms[i].pos);
            m_Atoms[i].sphere.setPosition(m_Atoms[i].pos);

            m_Atoms[i].force = sf::Vector2f(0.0f, 0.0f);
            m_Atoms[i].torque = 0.0f;
        }
    }
}



void
World::SpawnUpdate(const float delta)
{
    static float spawn = 0;

    if (spawn >= 0.1f)
    {
        sf::Vector2f pos = sf::Vector2f(10.0f + rand() % 1260, 10.0f + rand() % 700);
        AddAtom(AtomType::LIPID, 5.0f, 10.0f, pos, rand() % 10, sf::Color::Yellow);

        spawn = 0;
    }
    else
    {
        spawn += delta;
    }
}



void
World::RemoveUpdate()
{
    for (auto i = m_Atoms.begin(); i != m_Atoms.end(); ++i)
    {
        if (i->pos.x < 0 || i->pos.x > 1280 || i->pos.y < 0 || i->pos.y > 720)
        {
            i = m_Atoms.erase(i);
        }
    }
}



void
World::Draw( sf::RenderWindow& window )
{
    static sf::Vertex line[] =
    {
        sf::Vertex(sf::Vector2f(0, 0), sf::Color::White, sf::Vector2f(0, 0)),
        sf::Vertex(sf::Vector2f(0, 0), sf::Color::White, sf::Vector2f(0, 0))
    };

    for (int i = 0; i < m_Atoms.size(); i++)
    {
        window.draw(m_Atoms[i].ball);
        //window.draw(m_Atoms[i].sphere);

        line[0].position = m_Atoms[i].pos;
        line[1].position = m_Atoms[i].pos + 20.0f * g_scale * VectorFromAngle(m_Atoms[i].dir + M_PI);
        window.draw(line, 2, sf::Lines);
    }
}



void
World::AddAtom(const AtomType type, const float radius, const float interaction, const sf::Vector2f& pos, const float dir, const sf::Color& color)
{
    Atom atom;
    atom.type = type;
    atom.radius = radius * g_scale;
    atom.interaction = interaction * g_scale;
    atom.pos = pos;
    atom.velocity = sf::Vector2f(0, 0);
    atom.force = sf::Vector2f(0, 0);
    atom.dir = dir;
    atom.angular_vel = 0.0f;
    atom.torque = 0.0f;
    atom.ball.setRadius(atom.radius);
    atom.ball.setFillColor(color);
    atom.ball.setOrigin(atom.radius, atom.radius);
    atom.ball.setPosition(atom.pos);
    atom.sphere.setRadius(atom.interaction);
    atom.sphere.setFillColor(sf::Color::Transparent);
    atom.sphere.setOutlineThickness(1);
    atom.sphere.setOrigin(atom.interaction, atom.interaction);
    atom.sphere.setPosition(atom.pos);
    m_Atoms.push_back(atom);
}
