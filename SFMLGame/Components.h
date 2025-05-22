#pragma once

#include "Vec2.h"
#include "Animation.h"
#include<SFML/Graphics.hpp>

class Component
{
public:
	bool exists = false;
};

class CTransform : public Component
{
public:
	CTransform() = default;
	CTransform(const Vec2f& p)
		:pos(p) { }
	CTransform(const Vec2f& p, const Vec2f& s)
		:pos(p), scale(s) {}
	CTransform(const Vec2f& p, const Vec2f& v, const Vec2f& s, float a)
		:pos(p), velocity(v), scale(s), angle(a) {}

	Vec2f pos = { 0,0 };
	Vec2f prevPos = { 0,0 };
	Vec2f scale = { 1,1 };
	Vec2f velocity = { 0,0 };
	Vec2f facing = { 0.0, 1.0 };
	float angle = 0;
};

class CShape : public Component
{
public:
	CShape() = default;
	CShape(float radius, int points, const sf::Color& fill,
		const sf::Color& outline, float thickness)
		:circle(radius, points)
	{
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thickness);
		circle.setOrigin(radius, radius);
	}

	sf::CircleShape circle;
};

class CBoundingBox : public Component
{
public:
	CBoundingBox() = default;
	CBoundingBox(const Vec2f& s)
		:size(s), halfSize(s.x / 2, s.y / 2) {}
	CBoundingBox(const Vec2f& s, bool m, bool v)
		:size(s), halfSize(s.x / 2, s.y / 2), blockMove(m), blockVision(v) { }
	Vec2f size;
	Vec2f halfSize;
	bool blockMove = false;
	bool blockVision = false;
};

class CHealth : public Component
{
public:
	CHealth() = default;
	CHealth(int m, int c)
		: max(m), current(c) { }
	int max = 1;
	int current = 1;
};

class CInvincibility : public Component
{
public:
	CInvincibility() = default;
	CInvincibility(int f)
		: iframes(f) { }
	int iframes = 0;
};

class CDamage : public Component
{
public:
	CDamage() = default;
	CDamage(int d)
		:damage(d) { }
	int damage = 1;
};

class CAnimation : public Component
{
public:
	CAnimation() = default;
	CAnimation(const Animation& animation, bool l)
		:animation(animation), loop(l) {
	}
	Animation animation;
	bool loop = false;
};

class CGravity : public Component
{
public:
	CGravity() = default;
	CGravity(float g) : gravity(g) {}
	float gravity = 0;
};

class CState : public Component
{
public:
	CState() = default;
	CState(const string& s) : state(s) {}
	string state = "StandDown";
};

class CChasePlayer : public Component
{
public:
	CChasePlayer() = default;
	CChasePlayer(Vec2f p, float s)
		: home(p), speed(s) {}
	Vec2f home = { 0 ,0 };
	float speed = 0;
};

class CPatrol : public Component
{
public:
	CPatrol() = default;
	CPatrol(vector<Vec2f>& pos, float s)
		:positions(pos), speed(s) { }
	vector<Vec2f> positions;
	size_t currentPosition = 0;
	float speed = 0;
};

class CCollision : public Component
{
public:
	CCollision() = default;
	CCollision(float r)
		:radius(r) {}

	float radius = 0;
};

class CScore : public Component
{
public:
	CScore() = default;
	CScore(int s)
		:score(s) {
	}

	int score = 0;
};

class CLifespan : public Component
{
public:
	CLifespan() = default;
	CLifespan(int totalLifespan)
		:remaining(totalLifespan), lifespan(totalLifespan) {}

	int remaining = 0;
	int lifespan = 0;
};

class CInput : public Component
{
public:
	CInput() = default;
	CInput(int s)
		:speed(s) {
	}

	bool up = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool attack = false;
	bool canAttack = true;
	bool canTeleport = true;
	bool inTeleporter = false;
	bool canRun = true;
	int speed = 0;
	int teleportCooldown = 0;
	int bulletCooldown = 0;
	bool fromPlayer = true;
	int speedBuffTimer = 0;
	int strengthBuffTimer = 0;
	int invisibilityBuffTimer = 0;
	int invincibilityBuffTimer = 0;
};

class CDraggable : public Component
{
public:
	CDraggable() = default;

	bool dragging = false;
};