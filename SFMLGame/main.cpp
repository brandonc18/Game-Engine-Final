#include <SFML/Graphics.hpp>

#include "GameEngine.h"

int main()
{
	GameEngine game("assets.txt");
	game.run();

    return 0;
}