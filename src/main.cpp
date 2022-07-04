#include <cmath>
#include <cstdlib>

#include "World.h"



int main()
{
	const int gameWidth = 800;
	const int gameHeight = 600;

	// Create the window of the application
	sf::RenderWindow window(sf::VideoMode(gameWidth, gameHeight, 32), "BioSim v0.01", sf::Style::Titlebar | sf::Style::Close);
	window.setVerticalSyncEnabled(true);

	World world;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if ((event.type == sf::Event::Closed) || ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
			{
				window.close();
				break;
			}
		}

		world.Update();

		window.clear(sf::Color(50, 200, 50));
		world.Draw(window);
		window.display();
	}

	return EXIT_SUCCESS;
}
