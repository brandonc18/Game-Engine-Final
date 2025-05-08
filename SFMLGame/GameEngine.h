#pragma once

#include "Scene.h"
#include "Assets.h"
#include <SFML/Graphics.hpp>

using namespace std;

class GameEngine
{
public:
	GameEngine(const string& path);
	void update();
	void run();
	void quit();;
	void changeScene(const string& sceneName, Scene* scene, bool endCurrentScene = false);
	sf::RenderWindow& getWindow();
	Assets& getAssets();
	bool isRunning();
	void sUserInput();

private:
	void init(const string& path);
	Scene* getCurrentScene();

	sf::RenderWindow window;
	sf::Clock deltaClock;
	Assets assets;
	string currentScene;
	map<string, Scene*> sceneMap;
	bool running = true;
};