#include "Scene_Menu.h"
#include "Scene_Zelda.h"
#include "Scene_LevelEditor.h"
#include "GameEngine.h"
#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	:Scene(gameEngine)
{
	init();
}

void Scene_Menu::init()
{
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Space, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::E, "EDIT");

	title = "Strange Zelda";
	menuText.setCharacterSize(36);
	menuText.setFont(game->getAssets().getFont("Techfont"));

	menuStrings.push_back("Level 1");
	menuStrings.push_back("Level 2");
	menuStrings.push_back("Level 3");
	levelPaths.push_back("Level1.json");
	levelPaths.push_back("LevelEdited.json");
	levelPaths.push_back("LevelEditor.json");

	game->getAssets().getSound("Title").play();
	game->getAssets().getSound("Title").setLoop(1);
}

void Scene_Menu::update()
{
	sRender();
}

void Scene_Menu::sDoAction(const Action& action)
{
	if (action.getType() == "START")
	{
		if (action.getName() == "UP")
		{
			if (selectedMenuIndex > 0) { selectedMenuIndex--; }
			else { selectedMenuIndex = menuStrings.size() - 1; }
		}
		else if (action.getName() == "DOWN")
		{
			selectedMenuIndex = (selectedMenuIndex + 1) % menuStrings.size();
		}
		else if (action.getName() == "PLAY")
		{
			game->getAssets().getSound("Title").stop();
			game->changeScene("PLAY", new Scene_Zelda(game, levelPaths[selectedMenuIndex]));
		}
		else if (action.getName() == "EDIT") 
		{
			game->getAssets().getSound("Title").stop();
			game->changeScene("EDIT", new Scene_LevelEditor(game, "LevelEditor.json"));
		}
		else if (action.getName() == "QUIT")
		{
			onEnd();
		}
	}
}

void Scene_Menu::onEnd()
{
	game->quit();
}

void Scene_Menu::sRender()
{
	game->getWindow().setView(game->getWindow().getDefaultView());
	game->getWindow().clear(sf::Color(100, 65, 165));

	menuText.setString(title);
	menuText.setFillColor(sf::Color::Black);
	menuText.setPosition(sf::Vector2f(10, 10));

	for (int i = 0; i < menuStrings.size(); i++)
	{
		menuText.setString(menuStrings[i]);
		menuText.setPosition(sf::Vector2f(10, 110 + i * 72));
		if (i == selectedMenuIndex)
		{
			menuText.setFillColor(sf::Color::White);
		}
		else
		{
			menuText.setFillColor(sf::Color::Black);
		}

		game->getWindow().draw(menuText);
	}

	menuText.setCharacterSize(24);
	menuText.setFillColor(sf::Color::Black);
	menuText.setString("up:W, down: S, play: Space, edit:E, quit: Esc");
	ImGui::SFML::Render(game->getWindow());
	game->getWindow().display();
}