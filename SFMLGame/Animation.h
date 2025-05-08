#pragma once

#include<SFML/Graphics.hpp>
#include "Vec2.h"

using namespace std;

class Animation
{
public:

	Animation() = default;

	Animation(const string& name, sf::Texture& t)
		:Animation(name, t, 1, 0) { }

	Animation(const string& name, sf::Texture& t, size_t frameCount, size_t duration)
		:name(name), frameCount(frameCount), currentFrame(0), duration(duration)
	{
		size = Vec2f((float)t.getSize().x / frameCount, (float)t.getSize().y);
		sprite.setTexture(t);
		sprite.setOrigin(size.x / 2, size.y / 2);
		sprite.setTextureRect(sf::IntRect(floor(currentFrame) * size.x, 0, size.x, size.y));
		
	}

	//Updates the animation to show the new frame
	//Animation loops when it reaches the end
	void update()
	{
		if (currentFrame == duration)
		{
			if (currentAnimationFrame == frameCount - 1)
				currentAnimationFrame = 0;
			else
				currentAnimationFrame++;
			sprite.setTextureRect(sf::IntRect(floor(currentAnimationFrame) * size.x, 0, size.x, size.y));
			currentFrame = 0;
		}
		currentFrame++;
	}

	bool hasEnded() const
	{
		if (currentFrame == duration && currentAnimationFrame == frameCount - 1)
			return true;
		return false;
	}

	const string& getName() const
	{
		return name;
	}

	const Vec2f& getSize() const
	{
		return size;
	}

	const int getFrameCount() const
	{
		return frameCount;
	}

	sf::Sprite& getSprite()
	{
		return sprite;
	}
private:
	sf::Sprite sprite;
	int frameCount;
	int currentFrame = 0;
	int currentAnimationFrame = 0;
	int duration;
	Vec2f size;
	string name;
};
