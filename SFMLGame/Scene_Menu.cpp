#include "Scene_Menu.h"
#include "Scene_Zelda.h"
#include "Scene_LevelEditor.h"
#include "GameEngine.h"
#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "json.hpp"
using json = nlohmann::json;

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

	title = "Strange Dungeon";
	menuText.setCharacterSize(36);
	menuText.setFont(game->getAssets().getFont("Techfont"));

	menuStrings.push_back("Continue Level");
	menuStrings.push_back("Level 1");
	menuStrings.push_back("Level 2");
	menuStrings.push_back("Level 3");
	levelPaths.push_back("Continue.json");
	levelPaths.push_back("Level1.json");
	levelPaths.push_back("Level2.json");
	levelPaths.push_back("Menu.json");

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
			game->changeScene("EDIT", new Scene_LevelEditor(game, levelPaths[selectedMenuIndex]));
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

void Scene_Menu::loadMenuBackground() {
	entityManager = EntityManager();

	ifstream fin;
	fin.open("Menu.json");
	json j;
	fin >> j;

	for (int i = 0; i < j["entities"].size(); i++)
	{
		Entity* e = entityManager.addEntity(j["entities"][i]["tag"]);
		string animationName = j["entities"][i]["components"][1]["name"];
		e->add<CAnimation>(game->getAssets().getAnimation(animationName), true);
		e->add<CTransform>(Vec2f(j["entities"][i]["components"][0]["x"], j["entities"][i]["components"][0]["y"]), Vec2f(j["entities"][i]["components"][0]["scaleX"], j["entities"][i]["components"][0]["scaleY"]));
		e->get<CTransform>().prevPos = e->get<CTransform>().pos;
		if (j["entities"][i]["components"][2]["exists"])
		{
			e->add<CBoundingBox>(Vec2f(j["entities"][i]["components"][2]["x"], j["entities"][i]["components"][2]["y"]), j["entities"][i]["components"][2]["move"], j["entities"][i]["components"][2]["vision"]);
		}
		if (j["entities"][i]["components"][3]["exists"])
		{
			e->add<CHealth>(j["entities"][i]["components"][3]["max"], j["entities"][i]["components"][3]["current"]);
		}
		if (j["entities"][i]["components"][4]["exists"])
		{
			e->add<CDamage>(j["entities"][i]["components"][4]["damage"]);
		}
		if (j["entities"][i]["components"][5]["exists"])
		{
			e->add<CChasePlayer>(Vec2f(j["entities"][i]["components"][5]["home"][0], j["entities"][i]["components"][5]["home"][1]), j["entities"][i]["components"][5]["speed"]);
		}
		if (j["entities"][i]["components"][6]["exists"])
		{
			json positionsJson = j["entities"][i]["components"][6]["positions"];
			vector<Vec2f> positions;
			for (auto& pos : positionsJson)
			{
				positions.push_back(Vec2f(pos[0], pos[1]));
			}
			e->add<CPatrol>(positions, j["entities"][i]["components"][6]["speed"]);
		}
		if (j["entities"][i]["components"][7]["exists"])
		{
			e->add<CInput>(j["entities"][i]["components"][7]["speed"]);
		}
		if (e->tag() == "player")
			e->add<CState>();
		e->add<CDraggable>();
	}
}