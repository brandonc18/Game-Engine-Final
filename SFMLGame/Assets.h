#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

#include "Animation.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cassert>

using namespace std;

class Assets
{
public:
	Assets() = default;
	void loadFromFile(const string& path)
	{
		ifstream fin;
		fin.open(path);

		while (fin.good())
		{
			vector<string> configValues;
			string configLine, configValue;
			getline(fin, configLine);
			stringstream ss(configLine);
			while (!ss.eof())
			{
				getline(ss, configValue, ' ');
				configValues.push_back(configValue);
			}

			if (configValues[0] == "Texture")
			{
				addTexture(configValues[1], configValues[2]);
			}
			else if (configValues[0] == "Font")
			{
				addFont(configValues[1], configValues[2]);
			}
			else if (configValues[0] == "Animation")
			{
				addAnimation(configValues[1], getTexture(configValues[2]), stoi(configValues[3]), stoi(configValues[4]));
			}
			else if (configValues[0] == "Sound")
			{
				addSound(configValues[1], configValues[2]);
			}
		}
	}

	void addTexture(const string& name, const string& path)
	{
		sf::Texture texture;
		if (!texture.loadFromFile(path))
		{
			cerr << "Could not load texture " << name << endl;
		}

		textures[name] = texture;
	}

	void addAnimation(const string& name, sf::Texture& texture, int frameCount, int duration)
	{
		animations[name] = Animation(name, texture, frameCount, duration);
	}

	void addFont(const string& name, const string& path)
	{
		sf::Font font;
		if (!font.loadFromFile(path))
		{
			cerr << "Could not load font " << name << endl;
		}
		fonts[name] = font;
	}

	void addSound(const string& name, const string& path)
	{
		soundBuffers[name] = sf::SoundBuffer();
		if (!soundBuffers[name].loadFromFile(path))
		{
			cerr << "Could not load sound " << name << endl;
			soundBuffers.erase(name);
		}
		else
		{
			sounds[name] = sf::Sound(soundBuffers[name]);
			sounds[name].setVolume(25);
		}
	}

	sf::Texture& getTexture(const string& name) 
	{
		assert(textures.find(name) != textures.end());
		return textures.at(name);
	}

	Animation& getAnimation(const string& name)
	{
		assert(animations.find(name) != animations.end());
		return animations.at(name);
	}

	sf::Font& getFont(const string& name)
	{
		assert(fonts.find(name) != fonts.end());
		return fonts.at(name);
	}

	sf::Sound& getSound(const string& name)
	{
		assert(sounds.find(name) != sounds.end());
		return sounds.at(name);
	}

	map<string, sf::Texture>& getTextureMap()
	{
		return textures;
	}

	map<string, Animation>& getAnimationMap()
	{
		return animations;
	}

	map<string, sf::Font>& getFontMap()
	{
		return fonts;
	}

	map<string, sf::Sound>& getSoundMap()
	{
		return sounds;
	}

private:
	map<string, sf::Texture> textures;
	map<string, Animation> animations;
	map<string, sf::Font> fonts;
	map<string, sf::SoundBuffer> soundBuffers;
	map<string, sf::Sound> sounds;
};