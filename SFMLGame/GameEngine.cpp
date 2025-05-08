#include "GameEngine.h"
#include "Assets.h"
#include "Scene_Zelda.h"
#include "Scene_Menu.h"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include <iostream>

GameEngine::GameEngine(const string& path)
{
	init(path);
}

void GameEngine::init(const string& path)
{
	assets.loadFromFile(path);

	window.create(sf::VideoMode(1280, 768), "Assignment 4");
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);

	changeScene("MENU", new Scene_Menu(this));
}

Scene* GameEngine::getCurrentScene()
{
	return sceneMap[currentScene];
}

bool GameEngine::isRunning()
{
	return running && window.isOpen();
}

sf::RenderWindow& GameEngine::getWindow()
{
	return window;
}

Assets& GameEngine::getAssets()
{
	return assets;
}

void GameEngine::run()
{
	while (isRunning())
	{
		update();
		getCurrentScene()->update();
	}
}

void GameEngine::sUserInput()
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(window, event);

		if (event.type == sf::Event::Closed)
		{
			quit();
		}

		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::X)
			{
				sf::Texture texture;
				texture.create(window.getSize().x, window.getSize().y);
				texture.update(window);
				if (texture.copyToImage().saveToFile("test.png"))
				{
					std::cout << "Screenshot saved to test.png" << std::endl;
				}
			}
		}

		if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
		{
			//If current scene does not have an action associated with this key, skip event
			if (getCurrentScene()->getActionMap().find(event.key.code) == getCurrentScene()->getActionMap().end())
			{
				continue;
			}
			//Determine start or end action
			const string actionType = (event.type == sf::Event::KeyPressed) ? "START" : "END";

			//Lookup the action and send the action to the scene
			getCurrentScene()->doAction(Action(getCurrentScene()->getActionMap().at(event.key.code), actionType));
		}

		//if the mouse is over an ImGui window, don't process this mouse event
		if (ImGui::GetIO().WantCaptureMouse) { continue; }

		//Process mouse events
		if (event.type == sf::Event::MouseButtonPressed)
		{
			Vec2i mousePosition(event.mouseButton.x, event.mouseButton.y);
			switch (event.mouseButton.button)
			{
			case sf::Mouse::Left: { getCurrentScene()->doAction(Action("LEFT_CLICK", "START", mousePosition)); break; }
			case sf::Mouse::Middle: { getCurrentScene()->doAction(Action("MIDDLE_CLICK", "START", mousePosition)); break;}
			case sf::Mouse::Right: { getCurrentScene()->doAction(Action("RIGHT_CLICK", "START", mousePosition)); break;
			}
			default: break;
			}
		}

		if (event.type == sf::Event::MouseButtonReleased)
		{
			Vec2i mousePosition(event.mouseButton.x, event.mouseButton.y);
			switch (event.mouseButton.button)
			{
			case sf::Mouse::Left: { getCurrentScene()->doAction(Action("LEFT_CLICK", "END", mousePosition)); break; }
			case sf::Mouse::Middle: { getCurrentScene()->doAction(Action("MIDDLE_CLICK", "END", mousePosition)); break; }
			case sf::Mouse::Right: { getCurrentScene()->doAction(Action("RIGHT_CLICK", "END", mousePosition)); break;}
			default: break;
			}
		}
	}
}

void GameEngine::update()
{
	ImGui::SFML::Update(window, deltaClock.restart());
	sUserInput();
}

void GameEngine::quit()
{
	running = false;
	window.close();
}

void GameEngine::changeScene(const string& sceneName, Scene* scene, bool endCurrentScene)
{
	if (endCurrentScene)
		sceneMap[currentScene] = 0;
	currentScene = sceneName;
	sceneMap[sceneName] = scene;
}
