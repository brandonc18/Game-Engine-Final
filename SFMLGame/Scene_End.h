#pragma once
#include "Scene.h"

using namespace std;

class Scene_End : public Scene
{
public:
	Scene_End(GameEngine* gameEngine);
protected:
	void init();
	void update();
	void onEnd();
	void sRender();
	void sDoAction(const Action& action);
	void sAnimation();
	void loadMenuBackground();

	vector<string> menuStrings;
	vector<string> levelPaths;
	sf::Text menuText;
	sf::Text winText;
	string title;
	int selectedMenuIndex = 0;
};

