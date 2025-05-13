#include "Scene_LevelEditor.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "Assets.h"
#include "Components.h"
#include "GameEngine.h"
#include "Scene_Menu.h"
#include <SFML/OpenGL.hpp>
#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

Scene_LevelEditor::Scene_LevelEditor(GameEngine* gameEngine, const string& levelPath) : Scene(gameEngine), levelPath(levelPath) {
    init(levelPath);
}

void Scene_LevelEditor::init(const string& levelPath) {
    srand(time(NULL)); //seed all rand functions

    gridText.setCharacterSize(12);
    gridText.setFont(game->getAssets().getFont("Techfont"));
    game->getWindow().setTitle(levelPath);
    game->getAssets().getSound("Music").play();
    game->getAssets().getSound("Music").setLoop(1);

    // Register gameplay Actions

    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::Z, "SNAP");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    //registerAction(sf::Keyboard::Space, "ATTACK");
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
        if (e->has<CChasePlayer>())
        {
            auto& chase = e->get<CChasePlayer>();
            j["entities"][count]["components"].push_back({ {"exists", true}, {"type", "CChasePlayer"}, {"speed", chase.speed}, {"home", {chase.home.x, chase.home.y} } });
        }
        else
        {
            j["entities"][count]["components"].push_back({ {"exists", false}, { "type", "CChasePlayer" } });
        }
        if (e->has<CPatrol>())
        {
            auto& patrol = e->get<CPatrol>();
            json patrolConfig = { {"exists", true}, {"type", "CPatrol"}, {"speed", patrol.speed} };
            json positions = json::array();
            for (auto& pos : patrol.positions)
            {
                positions.push_back({ pos.x, pos.y });
            }
            patrolConfig["positions"] = positions;
            j["entities"][count]["components"].push_back(patrolConfig);
        }
        else
        {
            j["entities"][count]["components"].push_back({ {"exists", false}, { "type", "CPatrol" } });
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
    entityManager.update();

    if (!paused)
    {
        sCamera();
        sCreateEntity();
        sSelectEntity();
        sEntityGUI();
        sGUI();
        sRender();
    }
    else
    {
        sGUI();
        sRender();
    }
    currentFrame++;
}

void Scene_LevelEditor::sDoAction(const Action& action) {
    // Implement all actions described for game here
    // Only the setting of the player's input component variables should be set here
    // Do minimal logic in this function

    if (action.getType() == "START")
    {
        if (action.getName() == "TOGGLE_TEXTURE") { drawTextures = !drawTextures; }
        else if (action.getName() == "TOGGLE_COLLISION") { drawCollision = !drawCollision; }
        else if (action.getName() == "TOGGLE_GRID") { drawGrid = !drawGrid; }
        else if (action.getName() == "PAUSE") { setPaused(!paused); }
        else if (action.getName() == "SNAP") { snapToGrid = !snapToGrid; }
        else if (action.getName() == "QUIT") { onEnd(); }
        else if (action.getName() == "LEFT") { roomX -= 1; }
        else if (action.getName() == "RIGHT") { roomX += 1; }
        else if (action.getName() == "UP") { roomY -= 1; }
        else if (action.getName() == "DOWN") { roomY += 1; }
    }
    else if (action.getType() == "END"){}
}

void Scene_LevelEditor::sCreateEntity() {
    // Get mouse position in relation to the window
    Vec2i mousePixelPos = sf::Mouse::getPosition(game->getWindow());
    Vec2f mouseWorldPos = game->getWindow().mapPixelToCoords(mousePixelPos);

    // Destroy entity if new one is selected in ImGui
    if (selectedAnimationName != "none" && followMouse) {
        followMouse = false;
        movingEntity->destroy();
        movingEntity = nullptr; // Reset for the next entity
    }

    // Create new entity if selected
    if (selectedAnimationName != "none" && !followMouse) {
        selectedEntity = nullptr;
        followMouse = true;
        movingEntity = entityManager.addEntity("tile");
        movingEntity->add<CAnimation>(game->getAssets().getAnimation(selectedAnimationName), true);
        movingEntity->add<CBoundingBox>(Vec2f(movingEntity->get<CAnimation>().animation.getSize().x, movingEntity->get<CAnimation>().animation.getSize().y));
        if (snapToGrid) {
            movingEntity->add<CTransform>(getSnappedPosition(mouseWorldPos.x, mouseWorldPos.y));
        }
        else {
            movingEntity->add<CTransform>(Vec2f(mouseWorldPos.x, mouseWorldPos.y));
        }
    }

    // Selected Entity movement
    if (followMouse && movingEntity != nullptr) {
        if (snapToGrid) {
            movingEntity->get<CTransform>().pos = getSnappedPosition(mouseWorldPos.x, mouseWorldPos.y);
        }
        else {
            movingEntity->get<CTransform>().pos = Vec2f(mouseWorldPos.x, mouseWorldPos.y);
        }
    }
    // Remove selected animation as the entity is already created
    selectedAnimationName = "none";

    // Left Mouse click detection
    bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

    // Check to see if it should place
    if (isMousePressed && !wasLeftMousePressed && followMouse && movingEntity != nullptr && !ImGui::GetIO().WantCaptureMouse) {
        selectedAnimationName = movingEntity->get<CAnimation>().animation.getName();
        followMouse = false;
        movingEntity = nullptr; // Reset for the next entity
    }
    wasLeftMousePressed = isMousePressed; // Update state for next frame

    // Middle Mouse click detection
    isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Middle);

    // Check to see if it should unselect and unfollow
    if (isMousePressed && !wasMiddleMousePressed) {
        selectedAnimationName = "none";
        followMouse = false;
        if (movingEntity != nullptr) {
            movingEntity->destroy();
        }
        movingEntity = nullptr;
        selectedEntity = nullptr;
    }
    wasMiddleMousePressed = isMousePressed; // Update state for next frame
}

void Scene_LevelEditor::sSelectEntity() {
    // Get mouse position in relation to the window
    Vec2i mousePixelPos = sf::Mouse::getPosition(game->getWindow()); 
    Vec2f mouseWorldPos = game->getWindow().mapPixelToCoords(mousePixelPos);

    // Right Mouse click detection
    bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Right);

    // Check to see if it should place
    if (isMousePressed && !wasRightMousePressed && !followMouse && !ImGui::GetIO().WantCaptureMouse) {
        for (int i = 0; i < entityManager.getEntities().size(); i++) {
            Vec2f entityBBox = entityManager.getEntities()[i]->get<CAnimation>().animation.getSize();
            Vec2f entityBPos = entityManager.getEntities()[i]->get<CTransform>().pos;
            if ((entityBPos.x - entityBBox.x / 2) < mouseWorldPos.x && (entityBPos.x + entityBBox.x / 2) > mouseWorldPos.x
                && (entityBPos.y - entityBBox.y / 2) < mouseWorldPos.y && (entityBPos.y + entityBBox.y / 2) > mouseWorldPos.y) {
                selectedEntity = entityManager.getEntities()[i];
                break;
            }
        }
    }
    wasMiddleMousePressed = isMousePressed; // Update state for next frame
}

Vec2f Scene_LevelEditor::getSnappedPosition(float worldX, float worldY) const {
    // Calculate the room's base position
    float windowX = game->getWindow().getSize().x;
    float windowY = game->getWindow().getSize().y;

    float roomOffsetX = roomX * windowX;
    float roomOffsetY = roomY * windowY;

    // Convert world position to tile indices within the room
    int tileX = int(floor((worldX - roomOffsetX) / 64.0f));
    int tileY = int(floor((worldY - roomOffsetY) / 64.0f));

    // Calculate the snapped world position (center of the tile)
    float snappedX = roomOffsetX + tileX * 64.0f + 32.0f;
    float snappedY = roomOffsetY + tileY * 64.0f + 32.0f;

    return Vec2f(snappedX, snappedY);
}

void Scene_LevelEditor::sCamera() {
    // get current view, which we will modify in the if statement
    sf::View view = game->getWindow().getView();

    // calculate view for room-based camera
    Vec2f cameraCenter;

    //find the X mid pixel of the room
    int x = (roomX * game->getWindow().getSize().x) + (game->getWindow().getSize().x / 2);
    int y = (roomY * game->getWindow().getSize().y) + (game->getWindow().getSize().y / 2);
    //set the view
    view.setCenter(x, y);

    // then set the window view
    game->getWindow().setView(view);
}

void Scene_LevelEditor::sRender() {
    game->getWindow().clear(sf::Color(128, 0, 128));
    sf::RectangleShape tick({ 1.0f, 6.0f });
    tick.setFillColor(sf::Color::Black);

    // draw all Entity textures and animations
    if (drawTextures) {
        for (auto e : entityManager.getEntities()) {
            auto& transform = e->get<CTransform>();
            sf::Color c = sf::Color::White;

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

        for (auto e : entityManager.getEntities()) {
            auto& transform = e->get<CTransform>();

            if (e->has<CHealth>()) {
                auto& h = e->get<CHealth>();
                Vec2f size(64, 6);
                sf::RectangleShape rect({ size.x, size.y });
                rect.setPosition(transform.pos.x - 32, transform.pos.y - 48);
                rect.setFillColor(sf::Color(96, 96, 96));
                rect.setOutlineColor(sf::Color::Black);
                rect.setOutlineThickness(2);
                game->getWindow().draw(rect);

                float ratio = (float)(h.current) / h.max;
                size.x *= ratio;
                rect.setSize({ size.x, size.y });
                rect.setFillColor(sf::Color(255, 0, 0));
                rect.setOutlineThickness(0);
                game->getWindow().draw(rect);

                for (int i = 0; i < h.max; i++) {
                    tick.setPosition(rect.getPosition() + sf::Vector2f(i * 64 * 1 / h.max, 0));
                    game->getWindow().draw(tick);
                }
            }
        }
    }

    // draw the grid so that you can easily debug
    if (drawGrid) {
        float leftX = game->getWindow().getView().getCenter().x - width() / 2;
        float rightX = leftX + width() + gridSize.x;
        float topY = game->getWindow().getView().getCenter().y - height() / 2;
        float bottomY = topY + height() + gridSize.y;
        float nextGridX = leftX - ((int)leftX % (int)gridSize.x);
        float nextGridY = topY - ((int)topY % (int)gridSize.y);

        for (float x = nextGridX; x < rightX; x += gridSize.x) {
            drawLine(Vec2f(x, topY), Vec2f(x, bottomY));
        }

        for (float y = nextGridY; y < bottomY; y += gridSize.y) {
            drawLine(Vec2f(leftX, y), Vec2f(rightX, y));
            for (float x = nextGridX; x < rightX; x += gridSize.x) {
                string xCell = to_string((int)x / (int)gridSize.x);
                string yCell = to_string((int)y / (int)gridSize.y);
                gridText.setString("(" + xCell + "," + yCell + ")");
                gridText.setPosition(x + 3, y + 2);
                game->getWindow().draw(gridText);
            }
        }
    }

    // draw all Entity collision bounding boxes with a rectangleshape
    if (drawCollision) {
        sf::CircleShape dot(4);
        dot.setFillColor(sf::Color::Black);
        for (auto e : entityManager.getEntities()) {
            if (e->has<CBoundingBox>()) {
                auto& box = e->get<CBoundingBox>();
                auto& transform = e->get<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
                rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
                rect.setPosition(transform.pos.x, transform.pos.y);
                rect.setFillColor(sf::Color(0, 0, 0, 0));

                if (box.blockMove && box.blockVision) {
                    rect.setOutlineColor(sf::Color::Black);
                }
                if (box.blockMove && !box.blockVision) {
                    rect.setOutlineColor(sf::Color::Blue);
                }
                if (!box.blockMove && box.blockVision) {
                    rect.setOutlineColor(sf::Color::Red);
                }
                if (!box.blockMove && !box.blockVision) {
                    rect.setOutlineColor(sf::Color::White);
                }
                rect.setOutlineThickness(1);
                game->getWindow().draw(rect);
            }

            if (e->has<CPatrol>()) {
                auto& patrol = e->get<CPatrol>().positions;
                for (size_t p = 0; p < patrol.size(); p++) {
                    dot.setPosition(patrol[p].x, patrol[p].y);
                    game->getWindow().draw(dot);
                }
            }

        //    if (e->has<CChasePlayer>()) {
        //        sf::VertexArray lines(sf::LinesStrip, 2);
        //        lines[0].position.x = e->get<CTransform>().pos.x;
        //        lines[0].position.y = e->get<CTransform>().pos.y;
        //        lines[0].color = sf::Color::Black;
        //        lines[1].position.x = player()->get<CTransform>().pos.x;
        //        lines[1].position.y = player()->get<CTransform>().pos.y;
        //        lines[1].color = sf::Color::Black;
        //        game->getWindow().draw(lines);
        //        dot.setPosition(e->get<CChasePlayer>().home.x, e->get<CChasePlayer>().home.y);
        //        game->getWindow().draw(dot);
        //    }
        }
    }
    ImGui::SFML::Render(game->getWindow());
    game->getWindow().display();
}

void Scene_LevelEditor::sGUI() {
    // Not required but may help with debugging
    ImGui::Begin("Scene Properties");

    if (ImGui::BeginTabBar("MyTabBar")) {
        if (ImGui::BeginTabItem("Animations")) {
                ImGui::Indent();
                int count = 0;
                for (auto it = game->getAssets().getAnimationMap().begin(); it != game->getAssets().getAnimationMap().end();
                    ++it) {
                    count++;
                    sf::Sprite sprite = ((Animation)it->second).getSprite();
                    GLuint textureID = sprite.getTexture()->getNativeHandle();
                    ImTextureID imguiTextureID = (void*)(intptr_t)textureID;
                    if (ImGui::ImageButton(it->first.c_str(), imguiTextureID, ImVec2(64.0, 64.0), ImVec2(0, 0),
                        ImVec2(1.0 / ((Animation)it->second).getFrameCount(), 1.0))) {
                        selectedAnimationName = it->first; // Set the selected animation name
                    }
                    if ((count % 6) != 0) {
                        ImGui::SameLine();
                    }
                }
                ImGui::NewLine();
                ImGui::Checkbox("Snap to Grid", &snapToGrid);
                ImGui::Unindent();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Debug")) {
            ImGui::Checkbox("Draw Grid", &drawGrid);
            ImGui::Checkbox("Draw Textures", &drawTextures);
            ImGui::Checkbox("Draw Debug", &drawCollision);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Entity Manager"))
        {
            if (ImGui::TreeNode("Entities by Tag"))
            {
                for (int i = 0; i < 3; i++)
                {
                    //set tabtext to the correct name of our entities
                    string tabText = "";
                    if (i == 0) { tabText = "tile"; }
                    if (i == 1) { tabText = "npc"; }
                    if (i == 2) { tabText = "player"; }

                    // Use SetNextItemOpen() so set the default state of a node to be open. We could
                    // also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
                    if (i == 0)
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);

                    // Here we use PushID() to generate a unique base ID, and then the "" used as TreeNode id won't conflict.
                    // An alternative to using 'PushID() + TreeNode("", ...)' to generate a unique ID is to use 'TreeNode((void*)(intptr_t)i, ...)',
                    // aka generate a dummy pointer-sized value to be hashed. The demo below uses that technique. Both are fine.
                    ImGui::PushID(i);
                    if (ImGui::TreeNode(tabText.c_str()))
                    {
                        if (entityManager.getEntities(tabText).size() > 0)
                        {
                            for (int j = 0; j < entityManager.getEntities(tabText).size(); j++)
                            {
                                //create a string that will be pushed to the text char
                                string pushBackString = "";
                                int entityID = entityManager.getEntities()[j]->id();
                                //add the current value of total entities to the string
                                pushBackString += to_string(entityID);
                                //add the tag to the string
                                pushBackString += (" " + entityManager.getEntities(tabText)[j]->get<CAnimation>().animation.getName() + " (");
                                //push the current coordinates to the string
                                pushBackString += to_string(entityManager.getEntities(tabText)[j]->get<CTransform>().pos.x);
                                pushBackString += ", ";
                                pushBackString += to_string(entityManager.getEntities(tabText)[j]->get<CTransform>().pos.y);
                                pushBackString += ")";

                                //create a button to destroy the entity and show the ID, Tag, and Position
                                string stringID = to_string(j);
                                string header = "D##";
                                header += stringID;
                                stringID = header;
                                char const* cha = stringID.c_str();
                                if (ImGui::SmallButton(cha)) { entityManager.getEntities(tabText)[j]->destroy(); }
                                ImGui::SameLine();
                                ImGui::Text(pushBackString.c_str());
                            }
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("All Entities"))
            {
                if (entityManager.getEntities().size() > 0)
                {
                    for (int j = 0; j < entityManager.getEntities().size(); j++)
                    {
                        //create a string that will be pushed to the text char
                        string pushBackString = "";
                        int entityID = entityManager.getEntities()[j]->id();
                        //add the current value of total entities to the strig
                        pushBackString += to_string(entityID);
                        //add the tag to the string
                        pushBackString += (" " + entityManager.getEntities()[j]->get<CAnimation>().animation.getName() + " (");
                        //push the current coordinates to the string
                        pushBackString += to_string(entityManager.getEntities()[j]->get<CTransform>().pos.x);
                        pushBackString += ", ";
                        pushBackString += to_string(entityManager.getEntities()[j]->get<CTransform>().pos.y);
                        pushBackString += ")";

                        //create a button to destroy the entity and show the ID, Tag, and Position
                        string stringID = to_string(j);
                        string header = "D##";
                        header += stringID;
                        stringID = header;
                        char const* cha = stringID.c_str();
                        if (ImGui::SmallButton(cha)) { entityManager.getEntities()[j]->destroy(); }
                        ImGui::SameLine();
                        ImGui::Text(pushBackString.c_str());
                    }
                }
                ImGui::TreePop();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void Scene_LevelEditor::sEntityGUI() {
    if (selectedEntity != nullptr) {
        ImGui::Begin("Entity Properties");
        sf::Sprite sprite = selectedEntity->get<CAnimation>().animation.getSprite();
        GLuint textureID = sprite.getTexture()->getNativeHandle();
        ImTextureID imguiTextureID = (void*)(intptr_t)textureID;
        ImGui::ImageButton(imguiTextureID, ImVec2(64.0, 64.0), ImVec2(0, 0),
            ImVec2(1.0 / selectedEntity->get<CAnimation>().animation.getFrameCount(), 1.0));
        ImGui::SliderFloat("Bounding Box x", &selectedEntity->get<CBoundingBox>().size.x, 0, 120);
        ImGui::SameLine();
        ImGui::SliderFloat("Bounding Box y", &selectedEntity->get<CBoundingBox>().size.y, 0, 120);
        if (ImGui::Button("Delete Entity")) { 
            selectedEntity->destroy();
            selectedEntity = nullptr;
        }
        ImGui::End();
    }
}


void Scene_LevelEditor::onEnd() {
    game->getAssets().getSound("Music").stop();
    game->getAssets().getSound("Title").play();
    game->getAssets().getSound("Title").setLoop(1);
    //outputJSONFile();   
    game->changeScene("MENU", new Scene_Menu(game), true);
}

void Scene_LevelEditor::drawLine(Vec2f p1, Vec2f p2) {
    sf::Vertex line[] = { sf::Vertex(sf::Vector2f(p1.x, p1.y)), sf::Vertex(sf::Vector2f(p2.x, p2.y)) };

    game->getWindow().draw(line, 2, sf::Lines);
}

int Scene_LevelEditor::width() { return game->getWindow().getSize().x; }

int Scene_LevelEditor::height() { return game->getWindow().getSize().y; }

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