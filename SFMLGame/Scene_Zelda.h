#pragma once
#include "Scene.h"
class Scene_Zelda : public Scene
{
	struct PlayerConfig
	{
		float X, Y, BX, BY, SPEED, HEALTH;
		int HP, SPP, STP, INVINCP, INVISP;
	};
	struct NPCConfig
	{
		string N, AI;
		int RX, RY, TX, TY, H, D, NP;
		float S;
		bool BM, BV;
	};
	struct Tile {
		string N;
		int RX, RY, TX, TY;
		bool BM, BV, M;
	};

public:
	Scene_Zelda(GameEngine* gameEngine, const string& levelPath);
protected:
	void init(const string& levelPath);
	void update();
	Entity* player();
	void loadLevelJSON(const string& filename);
	void loadLevel(const string& filename);
	void saveLevel();
	void sDoAction(const Action& action);
	void patrolAI(Entity* npc);
	void chaseHelper(Entity* npc, Vec2f desiredPos);
	void chasePlayer(Entity* npc);
	void sAI();
	void sStatus();
	void sCamera();
	void sAnimation();
	void sMovement();
	void sDash();
	void sCollision();
	void sGUI();
	void sRender();
	void onEnd();

	Vec2f getPosition(int rx, int ry, int tx, int ty) const;
	void spawnPlayer();
	void spawnSword(Entity* entity);

	int width();
	int height();
	void drawLine(Vec2f p1, Vec2f p2);

	string levelPath;
	PlayerConfig playerConfig;
	Tile tileConfig;
	NPCConfig NPC;
	bool follow = true;
	bool drawTextures = true;
	bool drawCollision = false;
	bool drawGrid = false;
	bool hitPortalLastFrame = false;
	const Vec2f gridSize = { 64,64 };
	sf::Text gridText;
};

