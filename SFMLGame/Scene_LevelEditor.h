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
	void sEntityGUI();
	void sRender();
	void onEnd();
	void sCreateEntity();
	void sSelectEntity();
	Vec2f getSnappedPosition(float worldX, float worldY) const;

	int width();
	int height();
	void drawLine(Vec2f p1, Vec2f p2);

	string levelPath;
	bool follow = true;
	bool drawTextures = true;
	bool drawCollision = true;
	bool drawGrid = false;
	bool snapToGrid = false;
	bool followMouse = false;
	bool wasLeftMousePressed = false;
	bool wasMiddleMousePressed = false;
	bool wasRightMousePressed = false;
	const Vec2f gridSize = { 64,64 };
	int roomX = 0;
	int roomY = 0;
	Entity* movingEntity = nullptr;
	Entity* selectedEntity = nullptr;
	string selectedAnimationName = "none";
	sf::Text gridText;
};

