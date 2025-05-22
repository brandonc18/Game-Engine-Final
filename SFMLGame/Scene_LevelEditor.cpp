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
    loadLevelJSON(levelPath); 
    srand(time(NULL)); //seed all rand functions

    gridText.setCharacterSize(12);
    gridText.setFont(game->getAssets().getFont("Techfont"));
    game->getWindow().setTitle(levelPath);
    game->getAssets().getSound("Music").play();
    game->getAssets().getSound("Music").setLoop(1);

    // Register gameplay Actions

    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::Z, "SNAP");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::L, "SAVE");
}

void Scene_LevelEditor::loadLevelJSON(const string& filename)
{
    entityManager = EntityManager();

    ifstream fin;
    fin.open(filename);
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
	fout.open(levelPath);
	fout << j.dump(4);
	fout.close();
	cout << "Output JSON" << endl;
}

void Scene_LevelEditor::update() {
    entityManager.update();

    sAnimation();
    sCamera();
    sDragAndDrop();
    sEntityGUI();
    sGUI();
    sRender();

    currentFrame++;
}

void Scene_LevelEditor::sDoAction(const Action& action) {
    if (ImGui::GetIO().WantCaptureKeyboard && action.getName() != "QUIT") {
        return;
    }

    if (action.getType() == "START")
    {
        if (action.getName() == "TOGGLE_TEXTURE") { drawTextures = !drawTextures; }
        else if (action.getName() == "TOGGLE_COLLISION") { drawCollision = !drawCollision; }
        else if (action.getName() == "TOGGLE_GRID") { drawGrid = !drawGrid; }
        else if (action.getName() == "SNAP") { snapToGrid = !snapToGrid; }
        else if (action.getName() == "QUIT") { onEnd(); }
        else if (action.getName() == "LEFT") { roomX -= 1; }
        else if (action.getName() == "RIGHT") { roomX += 1; }
        else if (action.getName() == "UP") { roomY -= 1; }
        else if (action.getName() == "DOWN") { roomY += 1; }
        else if (action.getName() == "SAVE") { outputJSONFile(); }
    }
    else if (action.getType() == "END"){}
}

void Scene_LevelEditor::sDragAndDrop() {
    // Get mouse position in relation to the window
    Vec2i mousePixelPos = sf::Mouse::getPosition(game->getWindow());
    Vec2f mouseWorldPos = game->getWindow().mapPixelToCoords(mousePixelPos);
    bool isLeftMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    bool isRightMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Right);

    // Destroy entity if new one is selected in ImGui
    if (selectedEntity != nullptr) {
        if (selectedAnimationName != selectedEntity->get<CAnimation>().animation.getName() && ImGui::GetIO().WantCaptureMouse && selectedAnimation) {
            if (selectedEntity->get<CDraggable>().dragging == true) {
                selectedEntity->destroy();
            }
            selectedEntity = nullptr; // Reset for the next entity
        }
    }

    // Create new entity if selected
    if (selectedAnimationName != "none" && selectedAnimation || selectedAnimationName != "none" && continuePlacing && selectedEntity->get<CDraggable>().dragging == false) {
        followMouse = true;
        selectedAnimation = false;
        selectedEntity = entityManager.addEntity("tile");
        selectedEntity->add<CAnimation>(game->getAssets().getAnimation(selectedAnimationName), true);
        selectedEntity->add<CBoundingBox>(Vec2f(selectedEntity->get<CAnimation>().animation.getSize().x, selectedEntity->get<CAnimation>().animation.getSize().y));
        selectedEntity->add<CDraggable>().dragging = true;
        if (snapToGrid) {
            selectedEntity->add<CTransform>(getSnappedPosition(mouseWorldPos.x, mouseWorldPos.y));
        }
        else {
            selectedEntity->add<CTransform>(Vec2f(mouseWorldPos.x, mouseWorldPos.y));
        }
    }

    static bool leftPressed = false;

    for (auto e : entityManager.getEntities())
    {
        if (e->has<CDraggable>() && e->get<CDraggable>().dragging)
        {
            //Set e position to the current mouse position
            if (snapToGrid) {
                e->get<CTransform>().pos = getSnappedPosition(mouseWorldPos.x, mouseWorldPos.y);
            }
            else {
                e->get<CTransform>().pos = Vec2f(mouseWorldPos.x, mouseWorldPos.y);
            }
        }
        // Check to see if it should grab
        if (isLeftMousePressed && !leftPressed && !ImGui::GetIO().WantCaptureMouse && (selectedEntity == nullptr || selectedEntity->get<CDraggable>().dragging == false)) {
            if (isPointInside(e, mouseWorldPos)) {
                e->get<CDraggable>().dragging = true;
                selectedEntity = e;
                leftPressed = isLeftMousePressed;
            }
        }
        // Or if it should place
        else if (isLeftMousePressed && e->get<CDraggable>().dragging == true && !leftPressed && !ImGui::GetIO().WantCaptureMouse) {
            e->get<CDraggable>().dragging = false;
        }
        // Select entity without picking it back up
        if (isRightMousePressed && !ImGui::GetIO().WantCaptureMouse) {
            if ((isPointInside(e, mouseWorldPos))) {
                selectedEntity = e;
                break;
            }
        }
    }

    leftPressed = isLeftMousePressed;
}

bool Scene_LevelEditor::isPointInside(Entity* e, const Vec2f& point) {
    if (!e->has<CBoundingBox>()) return false;
    auto& box = e->get<CBoundingBox>();
    auto& transform = e->get<CTransform>();
    float left = transform.pos.x - box.size.x / 2;
    float right = transform.pos.x + box.size.x / 2;
    float top = transform.pos.y - box.size.y / 2;
    float bottom = transform.pos.y + box.size.y / 2;
    return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
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

void Scene_LevelEditor::sAnimation() {
    if (animate) {
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
                rect.setFillColor(sf::Color(0, 0, 255));
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
                        if (selectedEntity != nullptr && selectedEntity->get<CDraggable>().dragging) {
                            selectedEntity->destroy();
                            selectedEntity = nullptr;
                        }
                        selectedAnimation = true;
                        selectedAnimationName = it->first;
                    }
                    if ((count % 6) != 0) {
                        ImGui::SameLine();
                    }
                }
                ImGui::NewLine();
                ImGui::Checkbox("Snap to Grid", &snapToGrid);
                ImGui::Checkbox("Continue Placing", &continuePlacing);
                ImGui::Checkbox("Animate", &animate);
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

        char tagBuffer[256] = "";
        strcpy_s(tagBuffer, sizeof(tagBuffer), selectedEntity->tag().c_str());
        if (ImGui::InputText("Tag", tagBuffer, sizeof(tagBuffer))) {
            selectedEntity->setTag(tagBuffer); // Update the tag when changed
        }

        // Get list of animation names to change animation
        vector<string> animationNames;
        for (const auto& pair : game->getAssets().getAnimationMap()) {
            animationNames.push_back(pair.first);
        }
        if (selectedEntity->has<CAnimation>()) {
            auto& animation = selectedEntity->get<CAnimation>();
            ImGui::Text("Animation");
            string currentAnim = animation.animation.getName();
            if (ImGui::BeginCombo("Animation", currentAnim.c_str())) {
                for (const string& name : animationNames) {
                    bool isSelected = (currentAnim == name);
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        animation.animation = game->getAssets().getAnimation(name);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("Loop", &animation.loop);
        }

        // Position, Scale, Angle
        if (selectedEntity->has<CTransform>()) {
            auto& transform = selectedEntity->get<CTransform>();
            ImGui::Text("Transform");
            ImGui::DragFloat2("Position", &transform.pos.x, 0.1f);
            ImGui::DragFloat2("Scale", &transform.scale.x, 0.01f);
            ImGui::DragFloat("Angle", &transform.angle, 1.0f);
        }

        // Bounding Box Size, BlockMove, BlockVision
        if (selectedEntity->has<CBoundingBox>()) {
            auto& box = selectedEntity->get<CBoundingBox>();
            ImGui::Text("Bounding Box");
            if (ImGui::DragFloat2("Size", &box.size.x, 1.0f, 0.0f, 1000.0f)) {
                box.halfSize = Vec2f(box.size.x / 2, box.size.y / 2);
            }
            ImGui::Checkbox("Block Move", &box.blockMove);
            ImGui::Checkbox("Block Vision", &box.blockVision);
        }

        // Max and Current health
        if (selectedEntity->has<CHealth>()) {
            auto& health = selectedEntity->get<CHealth>();
            ImGui::Text("Health");
            ImGui::DragInt("Max Health", &health.max, 1, 1, 1000);
            ImGui::DragInt("Current Health", &health.current, 1, 0, health.max);
        }

        // Damage value
        if (selectedEntity->has<CDamage>()) {
            auto& damage = selectedEntity->get<CDamage>();
            ImGui::Text("Damage");
            ImGui::DragInt("Damage", &damage.damage, 1, 0, 1000);
        }

        // CChasePlayer Speed and Home position
        if (selectedEntity->has<CChasePlayer>()) {
            auto& chase = selectedEntity->get<CChasePlayer>();
            ImGui::Text("Chase Player");
            ImGui::DragFloat("Speed", &chase.speed, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat2("Home Position", &chase.home.x, 0.1f);
        }

        // CPatrol Speed and List of positions
        if (selectedEntity->has<CPatrol>()) {
            auto& patrol = selectedEntity->get<CPatrol>();
            ImGui::Text("Patrol");
            ImGui::DragFloat("Speed", &patrol.speed, 0.1f, 0.0f, 100.0f);
            for (size_t i = 0; i < patrol.positions.size(); ++i) {
                std::string label = "Position " + std::to_string(i);
                ImGui::DragFloat2(label.c_str(), &patrol.positions[i].x, 0.1f);
                ImGui::SameLine();
                if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) {
                    patrol.positions.erase(patrol.positions.begin() + i);
                    --i;
                }
            }
            if (ImGui::Button("Add Position")) {
                patrol.positions.push_back(Vec2f(0.0f, 0.0f));
            }
        }
        // Change Player speed
        if (selectedEntity->has<CInput>()) {
            auto& input = selectedEntity->get<CInput>();
            ImGui::Text("Input");
            ImGui::DragInt("Speed", &input.speed, 1, 0, 100);
        }

        ImGui::Separator();
        ImGui::Text("Add Component");

        // Add Health
        if (!selectedEntity->has<CHealth>()) {
            if (ImGui::Button("Add Health")) {
                selectedEntity->add<CHealth>(5, 5);
            }
        }
        // Add Damage
        if (!selectedEntity->has<CDamage>()) {
            if (ImGui::Button("Add Damage")) {
                selectedEntity->add<CDamage>(10);
            }
        }
        // Add CInput for player
        if (!selectedEntity->has<CInput>()) {
            if (ImGui::Button("Add Input")) {
                selectedEntity->add<CInput>();
            }
        }
        // Get position fo the Entity
        Vec2f currentPos(0.0f, 0.0f);
        if (selectedEntity->has<CTransform>()) {
            currentPos = selectedEntity->get<CTransform>().pos;
        }
        // Add Chase AI
        if (!selectedEntity->has<CChasePlayer>()) {
            if (ImGui::Button("Add Chase Player")) {
                selectedEntity->add<CChasePlayer>(currentPos, 1.0f); // Default speed 1.0
            }
        }
        // Add Patrol AI
        if (!selectedEntity->has<CPatrol>()) {
            if (ImGui::Button("Add Patrol")) {
                std::vector<Vec2f> patrolPositions = { currentPos, currentPos + Vec2f(10.0f, 0.0f) };
                selectedEntity->add<CPatrol>(patrolPositions, 1.0f);
            }
        }
        // Add Invincibility
        if (!selectedEntity->has<CInvincibility>()) {
            if (ImGui::Button("Add Invincibility")) {
                selectedEntity->add<CInvincibility>(60);
            }
        }
        // Destroy Entity
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
    outputJSONFile();  
    game->changeScene("MENU", new Scene_Menu(game), true);
}

void Scene_LevelEditor::drawLine(Vec2f p1, Vec2f p2) {
    sf::Vertex line[] = { sf::Vertex(sf::Vector2f(p1.x, p1.y)), sf::Vertex(sf::Vector2f(p2.x, p2.y)) };

    game->getWindow().draw(line, 2, sf::Lines);
}

int Scene_LevelEditor::width() { return game->getWindow().getSize().x; }

int Scene_LevelEditor::height() { return game->getWindow().getSize().y; }
