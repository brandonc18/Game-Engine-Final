#pragma once
#include "Scene.h"

class Scene_LevelEditor : public Scene
{
public:
	Scene_LevelEditor(GameEngine* gameEngine, const string& levelPath);
protected:
	void outputJSONFile();
	void init(const string& levelPath);
	void update();
	void sDoAction(const Action& action);
	void sGUI();
	void sRender();
	void onEnd();
};

