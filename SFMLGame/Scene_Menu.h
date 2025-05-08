#pragma once

#include "Scene.h"

using namespace std;

class Scene_Menu : public Scene
{
public:
	Scene_Menu(GameEngine* gameEngine);
protected:
	void init();
	void update();
	void onEnd();
	void sRender();
	void sDoAction(const Action& action);

	vector<string> menuStrings;
	vector<string> levelPaths;
	sf::Text menuText;
	string title;
	int selectedMenuIndex = 0;
};