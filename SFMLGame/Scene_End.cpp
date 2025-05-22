#include "Scene_End.h"
#include "Scene_Menu.h"
#include "Scene_Zelda.h"
#include "Scene_LevelEditor.h"
#include "GameEngine.h"
#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "json.hpp"
using json = nlohmann::json;

Scene_End::Scene_End(GameEngine* gameEngine)
	:Scene(gameEngine)
{
	init();
}

void Scene_End::init()
{
	loadMenuBackground();
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Space, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");

	title = "Strange Dungeon Finished";
	menuText.setCharacterSize(36);
	menuText.setFont(game->getAssets().getFont("Techfont"));

	winText.setFont(game->getAssets().getFont("Techfont"));
	winText.setString("You Won");
	winText.setCharacterSize(72);
	winText.setFillColor(sf::Color::Red);

	menuStrings.push_back("Main Menu");
	menuStrings.push_back("Quit");

	game->getAssets().getSound("Title").play();
	game->getAssets().getSound("Title").setLoop(1);
}

void Scene_End::update()
{
	entityManager.update();

	sAnimation();
	sRender();
	currentFrame++;
}

void Scene_End::sDoAction(const Action& action)
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
			if (menuStrings[selectedMenuIndex] == "Quit") {
				game->quit();
			}
			else {
				onEnd();
			}
		}
		else if (action.getName() == "QUIT")
		{
			onEnd();
		}
	}
}

void Scene_End::onEnd()
{
	game->getAssets().getSound("Music").stop();
	game->getAssets().getSound("Title").play();
	game->getAssets().getSound("Title").setLoop(1);
	game->changeScene("MENU", new Scene_Menu(game), true);
}

void Scene_End::sAnimation() {
	for (auto& entity : entityManager.getEntities()) {
		if (entity->has<CAnimation>()) {
			auto& animComponent = entity->get<CAnimation>();
			animComponent.animation.update();
			if (!animComponent.loop && animComponent.animation.hasEnded()) {
				entity->destroy();
			}
		}
	}
}

void Scene_End::sRender()
{
	game->getWindow().setView(game->getWindow().getDefaultView());
	game->getWindow().clear(sf::Color(100, 65, 165));

	float winTextWidth = winText.getLocalBounds().width;
	float windowWidth = game->getWindow().getSize().x;
	float xPos = (windowWidth - winTextWidth) / 2.0f;
	winText.setPosition(sf::Vector2f(xPos, 100.0f));
	game->getWindow().draw(winText);

	menuText.setString(title);
	menuText.setFillColor(sf::Color::Black);
	menuText.setPosition(sf::Vector2f(10, 10));

	for (auto e : entityManager.getEntities("tile")) {
		auto& transform = e->get<CTransform>();
		sf::Color c = sf::Color::White;

		if (paused) {
			c = sf::Color::Blue;
		}

		if (e->has<CAnimation>()) {
			auto& animation = e->get<CAnimation>().animation;
			animation.getSprite().setRotation(transform.angle);
			animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
			animation.getSprite().setScale(transform.scale.x, transform.scale.y);
			animation.getSprite().setColor(c);
			game->getWindow().draw(animation.getSprite());
		}
	}

	for (auto e : entityManager.getEntities()) {
		if (e->tag() != "tile") {
			auto& transform = e->get<CTransform>();
			sf::Color c = sf::Color::White;

			if (paused) {
				c = sf::Color::Blue;
			}
			if (e->has<CInvincibility>()) {
				c = sf::Color(255, 255, 255, 128);
			}

			if (e->has<CAnimation>()) {
				auto& animation = e->get<CAnimation>().animation;
				animation.getSprite().setRotation(transform.angle);
				animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
				animation.getSprite().setScale(transform.scale.x, transform.scale.y);
				animation.getSprite().setColor(c);
				game->getWindow().draw(animation.getSprite());
			}
		}
	}

	for (int i = 0; i < menuStrings.size(); i++)
	{
		menuText.setString(menuStrings[i]);
		float textWidth = menuText.getLocalBounds().width;
		float windowWidth = game->getWindow().getSize().x;
		float windowHeight = game->getWindow().getSize().y;
		float xPos = (windowWidth - textWidth) / 2.0f;
		float totalHeight = menuStrings.size() * 72.0f;
		float yPos = (windowHeight - totalHeight) / 2.0f + i * 72.0f;
		menuText.setPosition(sf::Vector2f(xPos, yPos));

		if (i == selectedMenuIndex)
		{
			menuText.setFillColor(sf::Color::Red);
		}
		else
		{
			menuText.setFillColor(sf::Color::White);
		}

		game->getWindow().draw(menuText);
		game->getWindow().draw(winText);
	}

	menuText.setCharacterSize(24);
	menuText.setFillColor(sf::Color::Black);
	menuText.setString("up:W, down: S, play: Space, edit:E, quit: Esc");
	ImGui::SFML::Render(game->getWindow());
	game->getWindow().display();
}

void Scene_End::loadMenuBackground() {
	entityManager = EntityManager();

	ifstream fin;
	fin.open("End.json");
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