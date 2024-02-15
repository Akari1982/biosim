#include <cmath>
#include <cstdlib>

#include "World.h"



int main()
{
    sf::RenderWindow window( sf::VideoMode( 1280, 720, 32 ), "BioSim v0.01", sf::Style::Titlebar | sf::Style::Close );
    window.setVerticalSyncEnabled( true );

    World world;

    while( window.isOpen() )
    {
        sf::Event event;
        while( window.pollEvent( event ) )
        {
            if( ( event.type == sf::Event::Closed ) || ( ( event.type == sf::Event::KeyPressed ) && ( event.key.code == sf::Keyboard::Escape ) ) )
            {
                window.close();
                break;
            }
        }

        world.Update();

        window.clear( sf::Color( 0, 0, 0 ) );
        world.Draw( window );
        window.display();
    }

    return EXIT_SUCCESS;
}
