#pragma once

#include <SFML/Graphics.hpp>

#include "Action.h"
#include "EntityManager.h"

using namespace std;

class GameEngine;

class Scene
{
public:
	Scene() = default;
	Scene(GameEngine* gameEngine)
		:game(gameEngine) { }
	virtual void update() = 0;
	virtual void sDoAction(const Action& action) = 0;
	virtual void sRender() = 0;

	void registerAction(int key, const string& name)
	{
		actionMap[key] = name;
	}

	void doAction(const Action& action)
	{
		sDoAction(action);
	}

	void simulate(int num)
	{
		for (int i = 0; i < num; i++)
		{
			update();
		}
	}

	map<int, string>& getActionMap()
	{
		return actionMap;
	}
protected:
	virtual void onEnd() = 0;
	void setPaused(bool paused)
	{
		this->paused = paused;
	}
	GameEngine* game = 0;
	EntityManager entityManager;
	map<int, string> actionMap;
	bool paused = false;
	size_t currentFrame = 0;
};
