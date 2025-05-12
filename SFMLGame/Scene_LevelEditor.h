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
	void sCamera();
	void sGUI();
	void sRender();
	void onEnd();
	void sCreateEntity();

	int width();
	int height();
	void drawLine(Vec2f p1, Vec2f p2);

	string levelPath;
	bool follow = true;
	bool drawTextures = true;
	bool drawCollision = false;
	bool drawGrid = false;
	bool hitPortalLastFrame = false;
	bool snapToGrid = false;
	const Vec2f gridSize = { 64,64 };
	int roomX = 0;
	int roomY = 0;
	string selectedAnimationName = "none";
	sf::Text gridText;
};

