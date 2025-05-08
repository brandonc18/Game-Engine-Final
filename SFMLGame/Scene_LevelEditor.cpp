#include "Scene_LevelEditor.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "Assets.h"
#include "Components.h"
#include "GameEngine.h"
//#include "Physics.h"
#include "Scene_Menu.h"
#include <SFML/OpenGL.hpp>
#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

Scene_LevelEditor::Scene_LevelEditor(GameEngine* gameEngine, const string& levelPath) {

}

void Scene_LevelEditor::init(const string& levelPath) {

}

void Scene_LevelEditor::outputJSONFile()
{
	int count = 0;
	json j;
	j["entities"] = json::array();
	for (auto& e : entityManager.getEntities())
	{
		j["entities"][count] = { { "id",e->id() }, {"tag", e->tag()} };
		j["entities"][count]["components"] = json::array();
		CTransform transform = e->get<CTransform>();
		j["entities"][count]["components"].push_back({ {"type", "CTransform"}, {"x", transform.pos.x}, {"y", transform.pos.y}, {"scaleX", transform.scale.x}, {"scaleY", transform.scale.y} });
		CAnimation animation = e->get<CAnimation>();
		j["entities"][count]["components"].push_back({ {"type", "CAnimation"}, {"name", animation.animation.getName()} });
		if (e->has<CBoundingBox>())
		{
			CBoundingBox box = e->get<CBoundingBox>();
			j["entities"][count]["components"].push_back({ {"exists", true}, { "type", "CBoundingBox" }, {"x", box.size.x}, {"y", box.size.y}, {"move", box.blockMove}, {"vision", box.blockVision} });
		}
		else
		{
			j["entities"][count]["components"].push_back({ {"exists", false}, { "type", "CBoundingBox" } });
		}
		if (e->has<CHealth>())
		{
			CHealth health = e->get<CHealth>();
			j["entities"][count]["components"].push_back({ {"exists", true}, {"type", "CHealth"}, {"max", health.max}, {"current", health.current} });
		}
		else
		{
			j["entities"][count]["components"].push_back({ {"exists", false}, { "type", "CHealth" } });
		}
		if (e->has<CDamage>())
		{
			CDamage damage = e->get<CDamage>();
			j["entities"][count]["components"].push_back({ {"exists", true}, {"type", "CDamage"}, {"damage", damage.damage} });
		}
		else
		{
			j["entities"][count]["components"].push_back({ {"exists", false}, { "type", "CDamage" } });
		}
		if (e->has<CInput>())
		{
			CInput input = e->get<CInput>();
			j["entities"][count]["components"].push_back({ {"exists", true}, {"type", "CInput"}, {"speed", input.speed} });
		}
		else
		{
			j["entities"][count]["components"].push_back({ {"exists", false}, { "type", "CInput" } });
		}
		count++;
	}

	ofstream fout;
	fout.open("LevelEditor.json");
	fout << j.dump(4);
	fout.close();
	cout << "Output JSON" << endl;
}

void Scene_LevelEditor::update() {

}

void Scene_LevelEditor::sDoAction(const Action& action) {

}

void Scene_LevelEditor::sGUI() {

}

void Scene_LevelEditor::sRender() {

}

void Scene_LevelEditor::onEnd() {

}

//void sInput()
//
//{
//
//	if (left mouse button clicked)
//
//	{
//
//		auto e = closestDraggableEntityToMouseClickPosition();
//
//		auto& d = e->getComponent<CDraggable>().dragging;
//
//		//If we are currently dragging, let go of entity
//
//		if (d) { d = false; }
//
//		//Otherwise if we clicked inside the entity, start dragging
//
//		else if (mouse click is inside e's animation) { d = true }
//
//	}
//
//}
//
//
//
//void sDragAndDrop()
//
//{
//
//	for (auto e : entities)
//
//	{
//
//		if (e->hasComponent<CDraggable>() && e->getComponent<CDraggable>().dragging)
//
//		{
//
//			//Set e position to the current mouse position
//
//		}
//
//	}
//
//}