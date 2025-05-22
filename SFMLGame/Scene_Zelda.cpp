#include "Scene_Zelda.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "Assets.h"
#include "Components.h"
#include "GameEngine.h"
#include "Physics.h"
#include "Scene_Menu.h"
#include <SFML/OpenGL.hpp>
#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"
#include <random>
#include <ctime>

#include"json.hpp"
using json = nlohmann::json;

using namespace std;

Scene_Zelda::Scene_Zelda(GameEngine *gameEngine, const string &levelPath) : Scene(gameEngine), levelPath(levelPath) {
  init(levelPath);
}

void Scene_Zelda::init(const string &levelPath) {

  loadLevelJSON(levelPath);

  srand(time(NULL)); //seed all rand functions

  gridText.setCharacterSize(12);
  gridText.setFont(game->getAssets().getFont("Techfont"));
  game->getWindow().setTitle(levelPath);
  game->getAssets().getSound("Music2").play();
  game->getAssets().getSound("Music2").setLoop(1);

  // Register gameplay Actions

  registerAction(sf::Keyboard::P, "PAUSE");
  registerAction(sf::Keyboard::Escape, "QUIT");
  registerAction(sf::Keyboard::Y, "TOGGLE_FOLLOW");
  registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
  registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
  registerAction(sf::Keyboard::G, "TOGGLE_GRID");
  registerAction(sf::Keyboard::A, "LEFT");
  registerAction(sf::Keyboard::D, "RIGHT");
  registerAction(sf::Keyboard::W, "UP");
  registerAction(sf::Keyboard::S, "DOWN");
  registerAction(sf::Keyboard::Space, "ATTACK");
}

void Scene_Zelda::loadLevelJSON(const string& filename)
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
        //if (animationName == "Black")
            //teleportTiles.push_back(e);
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
    }

    //instantiate the bullet config
    bulletConfig.S = 20;
    bulletConfig.L = 90;
    //player config for potions
    playerConfig.HP = 3;
    playerConfig.SPP = 3;
    playerConfig.STP = 3;
    playerConfig.INVINCP = 1;
}

void Scene_Zelda::loadLevel(const string &filename) {
  entityManager = EntityManager();
  //  read in the level file and add the appropriate entities
  //  use the PlayerConfig struct for player

  ifstream fin;
  fin.open(filename);
  string configLine;
  string tag;

  while (getline(fin, configLine)) {
      istringstream ss(configLine);
      ss >> tag;

      // setup config values and then create entities
      if (tag == "Player") {
          ss >> playerConfig.X >> playerConfig.Y >> playerConfig.BX >> playerConfig.BY >> playerConfig.SPEED >> playerConfig.HEALTH;
      }
      else if (tag == "NPC") {
          ss >> NPC.N >> NPC.RX >> NPC.RY >> NPC.TX >> NPC.TY >> NPC.BM >> NPC.BV >> NPC.H >> NPC.D >> NPC.AI >> NPC.S;

          Entity* enemy = entityManager.addEntity("npc");
          enemy->add<CAnimation>(game->getAssets().getAnimation(NPC.N), true);
          enemy->add<CTransform>(getPosition(NPC.RX, NPC.RY, NPC.TX, NPC.TY));
          enemy->get<CTransform>().prevPos = enemy->get<CTransform>().pos;
          enemy->add<CBoundingBox>(game->getAssets().getAnimation(NPC.N).getSize(), NPC.BM, NPC.BV);
          enemy->add<CHealth>(NPC.H, NPC.H);
          enemy->add<CDamage>(NPC.D);

          if (NPC.AI == "Chase") {
              Vec2f homePos = enemy->get<CTransform>().pos;
              enemy->add<CChasePlayer>(homePos, NPC.S);
          }
           else if (NPC.AI == "Patrol") {
              ss >> NPC.NP;
              vector<Vec2f> positions;
              int x, y;
              for (int i = 0; i < NPC.NP; i++) {
                  ss >> x >> y;
                  Vec2f position(x, y);
                  positions.push_back(getPosition(NPC.RX, NPC.RY, x, y));
              }
              enemy->add<CPatrol>(positions, NPC.S);
          }
          
      }
      else if (tag == "Tile") {
          ss >> tileConfig.N >> tileConfig.RX >> tileConfig.RY >> tileConfig.TX >> tileConfig.TY >> tileConfig.BM >> tileConfig.BV;

          Entity* tile = entityManager.addEntity("tile");
          tile->add<CAnimation>(game->getAssets().getAnimation(tileConfig.N), true);
          tile->add<CTransform>(getPosition(tileConfig.RX, tileConfig.RY, tileConfig.TX, tileConfig.TY));
          tile->get<CTransform>().prevPos = tile->get<CTransform>().pos;
          tile->add<CBoundingBox>(game->getAssets().getAnimation(tileConfig.N).getSize(), tileConfig.BM, tileConfig.BV);
      }
  }
  fin.close();


  //  read in the level file and add the appropriate entities
  //  use the getPosition() function below to convert room-tile coords to game world coords
  spawnPlayer();
}

void Scene_Zelda::saveLevel() {
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
    fout.open("Continue.json");
    fout << j.dump(4);
    fout.close();
    cout << "Saved Progress" << endl;
}

Vec2f Scene_Zelda::getPosition(int rx, int ry, int tx, int ty) const {
  //setup final position variable
  Vec2f position = { 0,0 };
  //set room additive pixels based on room number
  int windowX = game->getWindow().getSize().x;
  int windowY = game->getWindow().getSize().y;
  position.x += rx * windowX;
  position.y += ry * windowY;
  //add pixels to the position based on hte tile position
  position.x += tx * 64;
  position.y += ty * 64;
  //add pixels to set the position to the middle of the grid tile
  position.x += 32;
  position.y += 32;

  return position;
}

void Scene_Zelda::spawnPlayer() {
  // here is a sample player entity which you can use to contruct other entities
    playerConfig.X = player()->get<CTransform>().pos.x;
    playerConfig.Y = player()->get<CTransform>().pos.y;
    playerConfig.BY = 0;
    playerConfig.BX = 0;
    playerConfig.SPEED = player()->get<CInput>().speed;
  auto player = entityManager.addEntity("player");
  player->add<CAnimation>(game->getAssets().getAnimation("HeroIdleDown"), true);
  player->add<CTransform>(Vec2f(playerConfig.X, playerConfig.Y));
  player->get<CTransform>().prevPos = player->get<CTransform>().pos;
  player->add<CBoundingBox>(Vec2f(playerConfig.BX, playerConfig.BY), true, false);
  player->add<CHealth>(playerConfig.HEALTH, playerConfig.HEALTH);
  player->add<CState>().state = "HeroIdleDown";
  // Implement this function so that it uses the parameters input from the level file
  // Those parameters should be stored in playerConfig variable
}

void Scene_Zelda::spawnSword(Entity *entity) {
  // Implement the spawning of the sword, which:
  // Should be given appropriate lifespan
  // Should spawn at the appropriate location based on player's facing firection
  // Be given a damage value of 1
  // Should play the "Slash" sound   
    auto sword = entityManager.addEntity("sword");
    sword->add<CTransform>(entity->get<CTransform>().pos);
    auto& playerState = player()->get<CState>().state;
    
    // Find the direction the player is facing to determine attack direction
    // up
    if (playerState.find("Up") != string::npos) {
        player()->get<CState>().state = "HeroAttackBack";
        sword->get<CState>().state = "Longsword";
        sword->get<CTransform>().pos.y -= gridSize.x;
    }
    // left
    else if (player()->get<CTransform>().scale.x == -1 && playerState.find("Side") != string::npos) {
        sword->get<CTransform>().scale.x = -1;
        player()->get<CState>().state = "HeroAttackSide";
        sword->get<CState>().state = "LongswordSide";
        sword->get<CTransform>().pos.x -= gridSize.x;
    }
    // right
    else if (playerState.find("Right") != string::npos) {
        player()->get<CState>().state = "HeroAttackSide";
        sword->get<CState>().state = "LongswordSide";
        sword->get<CTransform>().pos.x += gridSize.y;
    }
    // down
    else {
        sword->get<CTransform>().scale.y = -1;
        player()->get<CState>().state = "HeroAttackFront";
        sword->get<CState>().state = "Longsword";
        sword->get<CTransform>().pos.y += gridSize.y;
    }
    sword->add<CAnimation>(game->getAssets().getAnimation(sword->get<CState>().state), true);
    sword->add<CBoundingBox>(game->getAssets().getAnimation(sword->get<CState>().state).getSize());
    sword->add<CLifespan>(10);
    sword->add<CDamage>(1);
    game->getAssets().getSound("Slash").play();
    player()->get<CInput>().canRun = false;
}

void Scene_Zelda::spawnBullet(Entity* entity)
{
    game->getAssets().getSound("Shoot").play();
    //TODO spawn bullet where mouse clicks using config values
    auto bullet = entityManager.addEntity("bullet");
    bullet->add<CTransform>(entity->get<CTransform>().pos);
    Vec2i mousePixelPos = sf::Mouse::getPosition(game->getWindow());
    Vec2f mouseWorldPos = game->getWindow().mapPixelToCoords(mousePixelPos);
    //find vector to give to bullet
    //	bullet speed is given as a scalar
    //	must set velocity by using formula in 2D math lecture
    //bullet if entity is the player
    if (entity->tag() == "player")
    {
        bullet->get<CInput>().fromPlayer = true;
        float speed = bulletConfig.S;
        Vec2f diffD = Vec2f(mouseWorldPos.x - player()->get<CTransform>().pos.x, mouseWorldPos.y - player()->get<CTransform>().pos.y);
        float distance = player()->get<CTransform>().pos.dist(mouseWorldPos);
        float cos = diffD.x / distance;
        float sin = diffD.y / distance;
        Vec2f velocity = Vec2f(speed * cos, speed * sin);
        bullet->get<CTransform>().velocity = velocity;
    }
    else if (entity->tag() == "npc")
    {
        bullet->get<CInput>().fromPlayer = false;
        float speed = bulletConfig.S;
        Vec2f diffD = Vec2f(player()->get<CTransform>().pos.x - entity->get<CTransform>().pos.x, player()->get<CTransform>().pos.y - entity->get<CTransform>().pos.y);
        float distance = entity->get<CTransform>().pos.dist(player()->get<CTransform>().pos);
        float cos = diffD.x / distance;
        float sin = diffD.y / distance;
        Vec2f velocity = Vec2f(speed * cos, speed * sin);
        bullet->get<CTransform>().velocity = velocity;
    }

    //initialize bullet
    bullet->get<CState>().state = "Bullet";
    bullet->add<CAnimation>(game->getAssets().getAnimation(bullet->get<CState>().state), true);
    bullet->add<CBoundingBox>(game->getAssets().getAnimation(bullet->get<CState>().state).getSize());
    bullet->add<CLifespan>(bulletConfig.L);
    bullet->add<CDamage>(1);
}

void Scene_Zelda::update() {
  entityManager.update();
  if (entityManager.getEntities("player").size() <= 0)
  {
      spawnPlayer();
  }

  if (!paused)
  {
      sAI();
      sMovement();
      sStatus();
      sCollision();
      sAnimation();
      sCamera();
      sGUI();
      sRender();
  }
  else
  {
      sGUI();
      sPausedGUI();
      sRender();
  }
  currentFrame++;
}

void Scene_Zelda::sMovement() {
  //create a velocity that will be used to check which direction the player is going
  Vec2f velocity = { 0,0 };
  //set player prev pos
  player()->get<CTransform>().prevPos = player()->get<CTransform>().pos;
  //check each input direction, make sure if the velocity for each direction is more than zero then
  //horizontal
  //left
  if (player()->get<CInput>().left && velocity.y == 0)
  {
      player()->get<CTransform>().scale.x = -1;
      if (!player()->get<CInput>().right && !player()->get<CInput>().attack && player()->get<CInput>().attack == false)
      {
          velocity.x = -3;
          player()->get<CState>().state = "HeroWalkSide";
      }
      else if (player()->get<CInput>().attack == false)
      {
          velocity.x = 0;
          player()->get<CState>().state = "HeroIdleSide";
      }
  }
  //right
  if (player()->get<CInput>().right && velocity.y == 0)
  {
      if (!player()->get<CInput>().left && !player()->get<CInput>().attack && player()->get<CInput>().attack == false)
      {
          velocity.x = 3;
          player()->get<CTransform>().scale.x = 1;
          player()->get<CState>().state = "HeroWalkSide";
      }
      else if (player()->get<CInput>().attack == false)
      {
          velocity.x = 0;
          player()->get<CState>().state = "HeroIdleSide";
      }
  }
  //vertical
  //up
  if (player()->get<CInput>().up && velocity.x == 0)
  {
      if (!player()->get<CInput>().down && !player()->get<CInput>().attack && player()->get<CInput>().attack == false)
      {
          velocity.y = -3;
          player()->get<CState>().state = "HeroWalkUp";
      }
      else if (player()->get<CInput>().attack == false)
      {
          velocity.y = 0;
          player()->get<CState>().state = "HeroIdleUp";
      }
  }
  //down
  if (player()->get<CInput>().down && velocity.x == 0)
  {
      if (!player()->get<CInput>().up && !player()->get<CInput>().attack && player()->get<CInput>().attack == false)
      {
          velocity.y = 3;
          player()->get<CState>().state = "HeroWalkDown";
      }
      else if (player()->get<CInput>().attack == false)
      {
          velocity.y = 0;
          player()->get<CState>().state = "HeroIdleDown";
      }
  }

  if (player()->get<CInput>().canRun == true) {
      player()->get<CTransform>().pos += velocity * player()->get<CInput>().speed;
  }

  //set states for idle animations when player is not moving
  if (velocity.x == 0 && velocity.y == 0)
  {
      if (player()->get<CAnimation>().animation.getName().find("Walk") != string::npos)
      {
          if (player()->get<CAnimation>().animation.getName() == "HeroWalkUp")
          {
              player()->get<CState>().state = "HeroIdleUp";
          }
          else if (player()->get<CAnimation>().animation.getName() == "HeroWalkDown")
          {
              player()->get<CState>().state = "HeroIdleDown";
          }
          else
          {
              player()->get<CState>().state = "HeroIdleSide";
          }
      }
  }

  //check if player is attacking, if so spawn the sword
  if (player()->get<CInput>().attack == true && player()->get<CInput>().canAttack == true)
  {
      spawnSword(player());
      player()->get<CInput>().canAttack = false;
  }

  //check if player is trying to teleport, if teleport is ready and it is a place they can teleport to, then teleport
  Vec2i mousePixelPos = sf::Mouse::getPosition(game->getWindow());
  Vec2f mouseWorldPos = game->getWindow().mapPixelToCoords(mousePixelPos);
  bool isRightMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Right);
  bool overlap;
  if (player()->get<CInput>().teleportCooldown > 0) { player()->get<CInput>().teleportCooldown--; }
  //check if player can teleport and is trying to teleport
  if (player()->get<CInput>().teleportCooldown <= 0 && isRightMousePressed && mouseWorldPos.dist(player()->get<CTransform>().pos) <= 320)
  {
      //check if the player is clicking on a tile that cannot be moved through
      for (int i = 0; i < entityManager.getEntities().size(); i++)
      {
          overlap = Physics::IsInside(mouseWorldPos, entityManager.getEntities()[i]);
          if (overlap)
          {
              if (entityManager.getEntities()[i]->get<CBoundingBox>().blockMove == true)
              {
                  cout << mouseWorldPos.x << " , " << mouseWorldPos.y << endl;
                  break;
              }
              else
              {
                  player()->get<CTransform>().pos = mouseWorldPos;
                  player()->get<CInput>().teleportCooldown = 180;
                  break;
              }
          }
      }
      if (!overlap)
      {
          player()->get<CTransform>().pos = mouseWorldPos;
          player()->get<CInput>().teleportCooldown = 180;
      }
  }

  player()->get<CInput>().bulletCooldown--;
  //shoot a bullet if the player presses the left mouse button
  bool isLeftMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
  if (isLeftMousePressed && player()->get<CInput>().bulletCooldown <= 0)
  {
      spawnBullet(player());
      player()->get<CInput>().bulletCooldown = 15;
  }

  //Movement for bullets
  for (int i = 0; i < entityManager.getEntities("bullet").size(); i++)
  {
      entityManager.getEntities("bullet")[i]->get<CTransform>().pos += entityManager.getEntities("bullet")[i]->get<CTransform>().velocity;
  }
}

void Scene_Zelda::sDoAction(const Action &action) {
  // Implement all actions described for game here
  // Only the setting of the player's input component variables should be set here
  // Do minimal logic in this function

  if (action.getType() == "START") 
  {
      if (action.getName() == "TOGGLE_TEXTURE") { drawTextures = !drawTextures; }
      else if (action.getName() == "TOGGLE_COLLISION") { drawCollision = !drawCollision; }
      else if (action.getName() == "TOGGLE_GRID") { drawGrid = !drawGrid; }
      else if (action.getName() == "PAUSE") { setPaused(!paused); }
      else if (action.getName() == "TOGGLE_FOLLOW") { follow = !follow; }
      else if (action.getName() == "QUIT") { onEnd(); }
      else if (action.getName() == "LEFT") { player()->get<CInput>().left = true; }
      else if (action.getName() == "RIGHT") { player()->get<CInput>().right = true; }
      else if (action.getName() == "UP") { player()->get<CInput>().up = true; }
      else if (action.getName() == "DOWN") { player()->get<CInput>().down = true; }
      else if (action.getName() == "ATTACK" && player()->get<CInput>().canAttack == true) { player()->get<CInput>().attack = true; }
  } 
  else if (action.getType() == "END") 
  {
    if (action.getName() == "LEFT") { player()->get<CInput>().left = false; }
    else if (action.getName() == "RIGHT") { player()->get<CInput>().right = false; }
    else if (action.getName() == "UP") { player()->get<CInput>().up = false; }
    else if (action.getName() == "DOWN") { player()->get<CInput>().down = false; }
  }
}

void Scene_Zelda::sAI() {
    //find the npcs and set their AI pattern
    for (auto npc : entityManager.getEntities("npc")) {
        if (npc->has<CPatrol>() && npc->has<CChasePlayer>()) {
            chasePatrolAI(npc);
        }
        else if (npc->has<CPatrol>()) {
            patrolAI(npc);
        }
        else if (npc->has<CChasePlayer>()) {
            chasePlayer(npc);
        }
    }
}

void Scene_Zelda::chasePatrolAI(Entity* npc) {
    bool canSee = true;
    // determine if there is anything blocking npc vision towards player
    if (npc->has<CChasePlayer>()) {
        Intersect npcplayer = Physics::EntityIntersect(npc->get<CTransform>().pos, player()->get<CTransform>().pos, player());
        float npcPlayerdist = npc->get<CTransform>().pos.dist(player()->get<CTransform>().pos);
        for (auto entity : entityManager.getEntities()) {
            if (npc->id() != entity->id() && entity->id() != player()->id()) {
                float npcObjectDistance = npc->get<CTransform>().pos.dist(entity->get<CTransform>().pos);
                if (npcObjectDistance < npcPlayerdist) {
                    Intersect object = Physics::EntityIntersect(npc->get<CTransform>().pos, player()->get<CTransform>().pos, entity);
                    if (object.result && entity->get<CBoundingBox>().blockVision) {
                        canSee = false;
                    }
                }
            }
        }
    }
    npc->get<CTransform>().prevPos = npc->get<CTransform>().pos;
    if (canSee) {   // if npc can see player, chase them
        chaseHelper(npc, player()->get<CTransform>().pos);
    }
    // If npc can't see player it patrols
    else {
        patrolAI(npc);
    }
}

void Scene_Zelda::patrolAI(Entity* npc) {
    // traverse the nodes
    auto& currentPos = npc->get<CTransform>().pos;
    int targetPos = npc->get<CPatrol>().currentPosition + 1;
    if (targetPos == npc->get<CPatrol>().positions.size()) { targetPos = 0; }
    // if not within 5 pixels move towards target in y
    if (abs(currentPos.y - npc->get<CPatrol>().positions[targetPos].y) >= 5) {
        if (currentPos.y < npc->get<CPatrol>().positions[targetPos].y) {
            currentPos.y += npc->get<CPatrol>().speed;
        }
        else {
            currentPos.y -= npc->get<CPatrol>().speed;
        }
    // if not within 5 pixels move towards target in x
    } else if (abs(currentPos.x - npc->get<CPatrol>().positions[targetPos].x) >= 5) {
        if (currentPos.x < npc->get<CPatrol>().positions[targetPos].x) {
            currentPos.x += npc->get<CPatrol>().speed;
        }
        else {
            currentPos.x -= npc->get<CPatrol>().speed;
        }
    // if target node is reached, set next target node
    } else { npc->get<CPatrol>().currentPosition = targetPos; }
}

void Scene_Zelda::chasePlayer(Entity* npc) {
    bool canSee = true;
    // determine if there is anything blocking npc vision towards player
    if (npc->has<CChasePlayer>()) {
        Intersect npcplayer = Physics::EntityIntersect(npc->get<CTransform>().pos, player()->get<CTransform>().pos, player());
        float npcPlayerdist = npc->get<CTransform>().pos.dist(player()->get<CTransform>().pos);
        for (auto entity : entityManager.getEntities()) {
            if (npc->id() != entity->id() && entity->id() != player()->id()) {
                float npcObjectDistance = npc->get<CTransform>().pos.dist(entity->get<CTransform>().pos);
                if (npcObjectDistance < npcPlayerdist) {
                    Intersect object = Physics::EntityIntersect(npc->get<CTransform>().pos, player()->get<CTransform>().pos, entity);
                    if (object.result && entity->get<CBoundingBox>().blockVision) {
                        canSee = false;
                    }
                }
            }
        }
    }
    npc->get<CTransform>().prevPos = npc->get<CTransform>().pos;
    if (canSee) {   // if npc can see player, chase them
        chaseHelper(npc, player()->get<CTransform>().pos);
    }
    // If npc can't see player, and isn't home, go home
    else if (abs(npc->get<CChasePlayer>().home.x - npc->get<CTransform>().pos.x) > 5 || abs(npc->get<CChasePlayer>().home.y - npc->get<CTransform>().pos.y) > 5) {
        chaseHelper(npc, npc->get<CChasePlayer>().home);
    }
}

void Scene_Zelda::chaseHelper(Entity* npc, Vec2f desiredPos) {
    Vec2f desired = desiredPos - npc->get<CTransform>().pos;
    float length;
    if (desired == Vec2f(0, 0)) {
        length = 0.1;   // prevent divide by zero
    }
    else {
        length = sqrt(desired.x * desired.x + desired.y * desired.y);
    }
    desired /= length;
    desired *= npc->get<CChasePlayer>().speed;
    npc->get<CTransform>().pos += desired;
}

void Scene_Zelda::sStatus() 
{
    //TODO add different potions and their effects
  
    // invinvibility frames
    if (!player()->get<CInvincibility>().exists) {}
    else if (player()->get<CInvincibility>().exists && player()->get<CInvincibility>().iframes > 0) { player()->get<CInvincibility>().iframes--; }
    else { player()->remove<CInvincibility>(); }
    // Lifespan frames
    for (int i = 0; i < entityManager.getEntities().size(); i++)
    {
        //if entity does not have a lifespan, do nothing
        if (!entityManager.getEntities()[i]->get<CLifespan>().exists) {}
        //if entity has a lifespan, check if it it has any remaining lifespan left
        else if (entityManager.getEntities()[i]->get<CLifespan>().exists)
        {
            //if entity has remaining lifespan, subtract from it, if not, destroy it
            if (entityManager.getEntities()[i]->get<CLifespan>().remaining > 0)
            {
                entityManager.getEntities()[i]->get<CLifespan>().remaining -= 1;
            }
            else if (entityManager.getEntities()[i]->get<CAnimation>().animation.getName().find("sword") != string::npos)
            {
                entityManager.getEntities()[i]->destroy();
                player()->get<CInput>().attack = false;
                player()->get<CInput>().canAttack = true;
                player()->get<CInput>().canRun = true;
                if (entityManager.getEntities()[i]->get<CAnimation>().animation.getName() == "LongswordDown")
                {
                    player()->get<CState>().state = "HeroIdleDown";
                }
                else if (entityManager.getEntities()[i]->get<CAnimation>().animation.getName() == "LongswordUp")
                {
                    player()->get<CState>().state = "HeroIdleUp";
                }
                else
                {
                    player()->get <CState>().state = "HeroIdleSide";
                }
            }
            else
            {
                entityManager.getEntities()[i]->destroy();
            }
        }
    }

    //Health Potion
    if (playerConfig.HP > 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) && player()->get<CHealth>().current < player()->get<CHealth>().max)
    {
        cout << "Health" << endl;
        playerConfig.HP--;
        player()->get<CHealth>().current = player()->get<CHealth>().max;
    }

    //Speed Potion
    if (player()->get<CInput>().speedBuffTimer <= 0) { player()->get<CInput>().speed = 1; }
    if (playerConfig.SPP > 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) && player()->get<CInput>().speedBuffTimer <= 0)
    {
        cout << "speed" << endl;
        player()->get<CInput>().speed = 2;
        player()->get<CInput>().speedBuffTimer = 300;
    }
    if (player()->get<CInput>().speedBuffTimer >= 0) { player()->get<CInput>().speedBuffTimer--; }

    //Strength Potion
    if (player()->get<CInput>().strengthBuffTimer <= 0) { player()->get<CInput>().strengthBuff = 0; }
    if (playerConfig.STP > 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::Num3) && player()->get<CInput>().strengthBuffTimer <= 0)
    {
        cout << "strength" << endl;
        player()->get<CInput>().strengthBuff = 1;
        player()->get<CInput>().strengthBuffTimer = 420;
    }
    if (player()->get<CInput>().strengthBuffTimer >= 0) { player()->get<CInput>().strengthBuffTimer--; }

    //Invincibility Potion
    if (playerConfig.INVINCP > 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::Num4) && !player()->get<CInvincibility>().exists)
    {
        cout << "invincibility" << endl;
        player()->add<CInvincibility>(600);
    }
}

void Scene_Zelda::sCollision() {
  //setup variable for player's collision box
  Vec2f entityABox;
  Vec2f entityAPos;
  //setup variables for the other entities collision box
  Vec2f entityBBox;
  Vec2f entityBPos;
  //get the overlap of the collision, then find which direction mario is going to know how to displace him
  //use overlap values to determine which way the player has entered the entity
  Vec2f overlap;
  Vec2f prevOverlap;
  //number of teleporters collided with
  int numOfTeleporters = 0;
  //check collisions for player
  for (int i = 0; i < entityManager.getEntities().size(); i++)
  {
      //setup variable for player's collision box
      entityABox = player()->get<CBoundingBox>().size;
      entityAPos = player()->get<CTransform>().pos;
      //setup variables for the other entities collision box
      entityBBox = entityManager.getEntities()[i]->get<CBoundingBox>().size;
      entityBPos = entityManager.getEntities()[i]->get<CTransform>().pos;
      //the collision bool will always start as false
      bool collides = false;
      //get the distance between the positions of the entities on the x and y axis
      float distanceX = abs(entityBPos.x - entityAPos.x);
      float distanceY = abs(entityBPos.y - entityAPos.y);
      //set overlaps here instead of only when it collides to set the num of tiles under the player
      overlap = Physics::GetOverlap(player(), entityManager.getEntities()[i]);
      prevOverlap = Physics::GetPreviousOverlap(player(), entityManager.getEntities()[i]);
      //if the is no collision
      if (overlap.x > 0 && overlap.y > 0 && entityManager.getEntities()[i]->tag() != "player" && entityManager.getEntities()[i]->get<CBoundingBox>().exists)
      {
          collides = true;
      }
      else {}

      //if the cant teleport is false, check to make sure no teleporters are collided with
      if (player()->get<CInput>().canTeleport == false)
      {
          vector <Entity*> teleporters;
          //capture all teleporters into a single vector to randomize in
          for (int j = 0; j < entityManager.getEntities().size(); j++)
          {
              //setup variable for player's collision box
              entityABox = player()->get<CBoundingBox>().size;
              entityAPos = player()->get<CTransform>().pos;
              //setup variables for the other entities collision box
              entityBBox = entityManager.getEntities()[j]->get<CBoundingBox>().size;
              entityBPos = entityManager.getEntities()[j]->get<CTransform>().pos;
              //the collision bool will always start as false
              bool collides = false;
              //get the distance between the positions of the entities on the x and y axis
              float distanceX = abs(entityBPos.x - entityAPos.x);
              float distanceY = abs(entityBPos.y - entityAPos.y);
              //set overlaps here instead of only when it collides to set the num of tiles under the player
              overlap = Physics::GetOverlap(player(), entityManager.getEntities()[j]);
              prevOverlap = Physics::GetPreviousOverlap(player(), entityManager.getEntities()[j]);
              //if the is no collision
              if (overlap.x > 0 && overlap.y > 0 && entityManager.getEntities()[j]->get<CAnimation>().animation.getName() == "Black" && entityManager.getEntities()[i]->get<CBoundingBox>().exists)
              {
                  teleporters.push_back(entityManager.getEntities()[j]);
              }
          }
          if (teleporters.size() <= 0) { player()->get<CInput>().canTeleport = true; }
      }

      //if the player collides with a tile, make a reolution on what it should do
      if (collides && entityManager.getEntities()[i]->tag() == "tile" && entityManager.getEntities()[i]->get<CBoundingBox>().blockMove == true || entityManager.getEntities()[i]->get<CAnimation>().animation.getName() == "DarkBrickStairs")
      {
          //vertical collision
          if (overlap.y > 0 && prevOverlap.x > 0)
          {
              //if player is going down into the tile and the absolute value of the X is greater than the Y
              if (entityBPos.y - entityAPos.y > 0)
              {
                  //nothing special is done for any tiles here, so just set the players position back to a point where they are not colliding
                  player()->get<CTransform>().pos.y -= overlap.y + 1;
                  player()->get<CTransform>().velocity.y = 0;
              }
              // if player is going up into the tile and the absolute value of the X is greater than the Y
              else
              {
                  //we just nudge the player back out of the collision
                  player()->get<CTransform>().velocity.y = 0;
                  player()->get<CTransform>().pos.y += overlap.y;
              }
          }

          //horizontal collision
          if (overlap.x > 0 && prevOverlap.y > 0)
          {
              //if player is going to the right into the tile and the absolute value of the Y is greater than the X
              if (entityBPos.x - entityAPos.x > 0)
              {
                  //nothing special is done for any tiles here, so just set the players position back to a point where they are not colliding
                  player()->get<CTransform>().velocity.x = 0;
                  player()->get<CTransform>().pos.x -= overlap.x + 1;
              }
              //if player is going to the left into the tile and the absolute value of the Y is greater than the X
              else
              {
                  //nothing special is done for any tiles here, so just set the players position back to a point where they are not colliding
                  player()->get<CTransform>().velocity.x = 0;
                  player()->get<CTransform>().pos.x += overlap.x;
              }
          }
      }

      //NPC collisions
      if (collides && entityManager.getEntities()[i]->tag() == "npc" && entityManager.getEntities()[i]->get<CAnimation>().animation.getName() != "DarkBrickStairs")
      {
          if (!player()->get<CInvincibility>().exists)
          {
              player()->add<CInvincibility>(30);
              player()->get<CHealth>().current--;
              game->getAssets().getSound("LinkHurt").play();
          }
          if (player()->get<CHealth>().current <= 0)
          {
              player()->destroy();
              game->getAssets().getSound("Death").play();
          }
      }

      //portal collisions
      if (collides && entityManager.getEntities()[i]->tag() == "tile" && entityManager.getEntities()[i]->get<CBoundingBox>().blockMove == false)
      {
          if (entityManager.getEntities()[i]->get<CAnimation>().animation.getName() == "Black")
          {
              vector <Entity*> teleporters;
              //capture all teleporters into a single vector to randomize in
              for (int j = 0; j < entityManager.getEntities().size(); j++)
              {
                  if (entityManager.getEntities()[j]->get<CAnimation>().animation.getName() == "Black")
                  {
                      teleporters.push_back(entityManager.getEntities()[j]);
                  }
              }
              //randomize a number based on the amount of teleporters in the vector
              int teleSpawn = rand() % teleporters.size();
              if (teleporters[teleSpawn]->id() != entityManager.getEntities()[i]->id() && player()->get<CInput>().canTeleport == true)
              {
                  player()->get<CTransform>().pos = teleporters[teleSpawn]->get<CTransform>().pos;
                  player()->get<CInput>().canTeleport = false;
              }
              //if the randomized teleporter is the same as the one the player is at, randomize to a new one until it is not the same
              else if (player()->get<CInput>().canTeleport == true)
              {
                  while (teleporters[teleSpawn]->id() == entityManager.getEntities()[i]->id())
                  {
                      teleSpawn = rand() % teleporters.size();
                  }
                  player()->get<CTransform>().pos = teleporters[teleSpawn]->get<CTransform>().pos;
                  player()->get<CInput>().canTeleport = false;
              }
              //set the portal hit last frame to true
              hitPortalLastFrame = true;
              numOfTeleporters++;
          }
          //if the player does not have full health, restore some when touching a heart tile
          if (entityManager.getEntities()[i]->get<CAnimation>().animation.getName() == "Heart" && player()->get<CHealth>().current < player()->get<CHealth>().max)
          {
              entityManager.getEntities()[i]->destroy();
              player()->get<CHealth>().current++;
              game->getAssets().getSound("GetHeart").play();
          }
      }
  }

  //Collision checks for Bullets
  for (int i = 0; i < entityManager.getEntities().size(); i++)
  {
      if (entityManager.getEntities()[i]->tag() == "bullet")
      {
          //check collisions
          for (int j = 0; j < entityManager.getEntities().size(); j++)
          {
              //setup variable for player's collision box
              entityABox = entityManager.getEntities()[i]->get<CBoundingBox>().size;
              entityAPos = entityManager.getEntities()[i]->get<CTransform>().pos;
              //setup variables for the other entities collision box
              entityBBox = entityManager.getEntities()[j]->get<CBoundingBox>().size;
              entityBPos = entityManager.getEntities()[j]->get<CTransform>().pos;
              //the collision bool will always start as false
              bool collides = false;
              //get the distance between the positions of the entities on the x and y axis
              float distanceX = abs(entityBPos.x - entityAPos.x);
              float distanceY = abs(entityBPos.y - entityAPos.y);
              //set overlaps here instead of only when it collides to set the num of tiles under the player
              overlap = Physics::GetOverlap(entityManager.getEntities()[i], entityManager.getEntities()[j]);
              prevOverlap = Physics::GetPreviousOverlap(entityManager.getEntities()[i], entityManager.getEntities()[j]);
              if (overlap.x > 0 && overlap.y > 0 && entityManager.getEntities()[j]->tag() != "bullet" && entityManager.getEntities()[j]->get<CBoundingBox>().exists)
              {
                  collides = true;
              }
              else {}

              if (collides && entityManager.getEntities()[j]->tag() == "tile" && entityManager.getEntities()[j]->get<CBoundingBox>().blockMove == true)
              {
                  entityManager.getEntities()[i]->destroy();
              }
              else if (collides && entityManager.getEntities()[j]->tag() == "npc" && entityManager.getEntities()[i]->get<CInput>().fromPlayer == true)
              {
                  if (entityManager.getEntities()[j]->get<CHealth>().exists && entityManager.getEntities()[i]->get<CDamage>().exists)
                  {
                      entityManager.getEntities()[j]->get<CHealth>().current -= (entityManager.getEntities()[i]->get<CDamage>().damage + player()->get<CInput>().strengthBuff);
                      entityManager.getEntities()[i]->get<CDamage>().exists = false;
                      entityManager.getEntities()[i]->destroy();
                      game->getAssets().getSound("EnemyHit").play();
                  }
                  if (entityManager.getEntities()[j]->get<CHealth>().current <= 0)
                  {
                      entityManager.getEntities()[j]->destroy();
                      game->getAssets().getSound("Death").play();
                  }
              }
              else if (collides && entityManager.getEntities()[j]->tag() == "player" && entityManager.getEntities()[i]->get<CInput>().fromPlayer == false)
              {
                  if (entityManager.getEntities()[j]->get<CHealth>().exists && entityManager.getEntities()[i]->get<CDamage>().exists)
                  {
                      entityManager.getEntities()[j]->get<CHealth>().current -= entityManager.getEntities()[i]->get<CDamage>().damage;
                      entityManager.getEntities()[i]->get<CDamage>().exists = false;
                      entityManager.getEntities()[i]->destroy();
                      game->getAssets().getSound("EnemyHit").play();
                  }
                  if (entityManager.getEntities()[j]->get<CHealth>().current <= 0)
                  {
                      entityManager.getEntities()[j]->destroy();
                      game->getAssets().getSound("Death").play();
                  }
              }
          }
      }
  }

  //Collision checks for NPC's
  for (int i = 0; i < entityManager.getEntities().size(); i++)
  {
      if (entityManager.getEntities()[i]->tag() == "npc")
      {
          //check collisions for npcs
          for (int j = 0; j < entityManager.getEntities().size(); j++)
          {
              //setup variable for player's collision box
              entityABox = entityManager.getEntities()[i]->get<CBoundingBox>().size;
              entityAPos = entityManager.getEntities()[i]->get<CTransform>().pos;
              //setup variables for the other entities collision box
              entityBBox = entityManager.getEntities()[j]->get<CBoundingBox>().size;
              entityBPos = entityManager.getEntities()[j]->get<CTransform>().pos;
              //the collision bool will always start as false
              bool collides = false;
              //get the distance between the positions of the entities on the x and y axis
              float distanceX = abs(entityBPos.x - entityAPos.x);
              float distanceY = abs(entityBPos.y - entityAPos.y);
              //set overlaps here instead of only when it collides to set the num of tiles under the player
              overlap = Physics::GetOverlap(entityManager.getEntities()[i], entityManager.getEntities()[j]);
              prevOverlap = Physics::GetPreviousOverlap(entityManager.getEntities()[i], entityManager.getEntities()[j]);
              if (overlap.x > 0 && overlap.y > 0 && entityManager.getEntities()[j]->tag() != "npc" && entityManager.getEntities()[j]->get<CBoundingBox>().exists)
              {
                  collides = true;
              }
              else {}

              //if the NPC collides with a tile, make a resolution on what it should do
              if (collides && entityManager.getEntities()[j]->tag() == "tile" && entityManager.getEntities()[j]->get<CBoundingBox>().blockMove == true)
              {
                  //vertical collision
                  if (overlap.y > 0 && prevOverlap.x > 0)
                  {
                      //if player is going down into the tile and the absolute value of the X is greater than the Y
                      if (entityBPos.y - entityAPos.y > 0)
                      {
                          //nothing special is done for any tiles here, so just set the players position back to a point where they are not colliding
                          entityManager.getEntities()[i]->get<CTransform>().pos.y -= overlap.y + 1;
                      }
                      // if player is going up into the tile and the absolute value of the X is greater than the Y
                      else
                      {
                          //we just nudge the player back out of the collision
                          entityManager.getEntities()[i]->get<CTransform>().pos.y += overlap.y;
                      }
                  }

                  //horizontal collision
                  if (overlap.x > 0 && prevOverlap.y > 0)
                  {
                      //if player is going to the right into the tile and the absolute value of the Y is greater than the X
                      if (entityBPos.x - entityAPos.x > 0)
                      {
                          //nothing special is done for any tiles here, so just set the players position back to a point where they are not colliding
                          entityManager.getEntities()[i]->get<CTransform>().pos.x -= overlap.x + 1;
                      }
                      //if player is going to the left into the tile and the absolute value of the Y is greater than the X
                      else
                      {
                          //nothing special is done for any tiles here, so just set the players position back to a point where they are not colliding
                          entityManager.getEntities()[i]->get<CTransform>().pos.x += overlap.x;
                      }
                  }
              }
          }
      }
  }

  //Collision checks for Sword
  for (int i = 0; i < entityManager.getEntities().size(); i++)
  {
      if (entityManager.getEntities()[i]->get<CAnimation>().animation.getName().find("Sword") != string::npos)
      {
          //check collisions for player
          for (int j = 0; j < entityManager.getEntities().size(); j++)
          {
              //setup variable for player's collision box
              entityABox = entityManager.getEntities()[i]->get<CBoundingBox>().size;
              entityAPos = entityManager.getEntities()[i]->get<CTransform>().pos;
              //setup variables for the other entities collision box
              entityBBox = entityManager.getEntities()[j]->get<CBoundingBox>().size;
              entityBPos = entityManager.getEntities()[j]->get<CTransform>().pos;
              //the collision bool will always start as false
              bool collides = false;
              //get the distance between the positions of the entities on the x and y axis
              float distanceX = abs(entityBPos.x - entityAPos.x);
              float distanceY = abs(entityBPos.y - entityAPos.y);
              //set overlaps here instead of only when it collides to set the num of tiles under the player
              overlap = Physics::GetOverlap(entityManager.getEntities()[i], entityManager.getEntities()[j]);
              prevOverlap = Physics::GetPreviousOverlap(entityManager.getEntities()[i], entityManager.getEntities()[j]);
              //if the is no collision, the player is in the air
              if (overlap.x > 0 && overlap.y > 0 && entityManager.getEntities()[j]->tag() == "npc" && entityManager.getEntities()[j]->get<CBoundingBox>().exists)
              {
                  collides = true;
              }
              else {}

              //if the NPC collides with a tile, make a resolution on what it should do
              if (collides)
              {
                  if (entityManager.getEntities()[j]->get<CHealth>().exists && entityManager.getEntities()[i]->get<CDamage>().exists)
                  {
                      entityManager.getEntities()[j]->get<CHealth>().current -= (entityManager.getEntities()[i]->get<CDamage>().damage + player()->get<CInput>().strengthBuff);
                      entityManager.getEntities()[i]->get<CDamage>().exists = false;
                      game->getAssets().getSound("EnemyHit").play();
                  }
                  if (entityManager.getEntities()[j]->get<CHealth>().current <= 0)
                  {
                      entityManager.getEntities()[j]->destroy();
                      game->getAssets().getSound("Death").play();
                  }
              }
          }
      }
  }
}

void Scene_Zelda::sAnimation() {
  // TODO Implement player facing direction animation
  // Implement sword animation based on player facing
  //	The sword should move if the player changes direction mid swing
  // Implement destruction of entities with non-looping finished animations
  for (auto& entity : entityManager.getEntities()) {
    if (entity->has<CAnimation>()) {
        auto& animComponent = entity->get<CAnimation>();
        animComponent.animation.update();
        if (!animComponent.loop && animComponent.animation.hasEnded()) {
            entity->destroy();
        }
    }
  }
  if (player()->get<CAnimation>().animation.getName() != player()->get<CState>().state) {
      player()->add<CAnimation>(game->getAssets().getAnimation(player()->get<CState>().state), true);
  }
}

void Scene_Zelda::sCamera() {
  // maybe kyle math hard
  // To Bingen: Kyle figured it out :)

  // get current view, which we will modify in the if statement
  sf::View view = game->getWindow().getView();

  if (follow) {
    // calculate view for player follow
    view.setCenter(player()->get<CTransform>().pos);
  } else {
    // calculate view for room-based camera
    Vec2f cameraCenter;
    //get the pixel position for the X axis, start by getting the room the player is in
    float roomX = player()->get<CTransform>().pos.x / game->getWindow().getSize().x;
    if (roomX < 0) { roomX -= 1; }
    roomX = trunc(roomX);
    //find the X mid pixel of the room
    roomX = (roomX * game->getWindow().getSize().x) + (game->getWindow().getSize().x / 2);

    //get the pixel position for the Y axis, start by getting the room the player is in
    float roomY = player()->get<CTransform>().pos.y / game->getWindow().getSize().y;
    if (roomY < 0) { roomY -= 1; }
    roomY = trunc(roomY);
    //find the Y mid pixel of the room
    roomY = (roomY * game->getWindow().getSize().y) + (game->getWindow().getSize().y / 2);
    //set the view
    view.setCenter(roomX, roomY);
  }
  // then set the window view
  game->getWindow().setView(view);
}

void Scene_Zelda::onEnd() {
  // Implement scene end
  // stop music
  // play menu scene music
  // change scene to menu
  game->getAssets().getSound("Music2").stop();
  game->getAssets().getSound("Music2").play();
  game->getAssets().getSound("Music2").setLoop(1);
  game->changeScene("MENU", new Scene_Menu(game), true);
}

void Scene_Zelda::sRender() {
  if (!paused) {
    game->getWindow().clear(sf::Color(255, 192, 122));
  }
  else {
    game->getWindow().clear(sf::Color(50, 50, 130));
  }
  sf::RectangleShape tick({1.0f, 6.0f});
  tick.setFillColor(sf::Color::Black);

  // draw all Entity textures and animations
  if (drawTextures) {

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

    for (auto e : entityManager.getEntities()) {
      auto &transform = e->get<CTransform>();

      if (e->has<CHealth>()) {
        auto &h = e->get<CHealth>();
        Vec2f size(64, 6);
        sf::RectangleShape rect({size.x, size.y});
        rect.setPosition(transform.pos.x - 32, transform.pos.y - 48);
        rect.setFillColor(sf::Color(96, 96, 96));
        rect.setOutlineColor(sf::Color::Black);
        rect.setOutlineThickness(2);
        game->getWindow().draw(rect);

        float ratio = (float)(h.current) / h.max;
        size.x *= ratio;
        rect.setSize({size.x, size.y});
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
        auto &box = e->get<CBoundingBox>();
        auto &transform = e->get<CTransform>();
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
        auto &patrol = e->get<CPatrol>().positions;
        for (size_t p = 0; p < patrol.size(); p++) {
          dot.setPosition(patrol[p].x, patrol[p].y);
          game->getWindow().draw(dot);
        }
      }

      if (e->has<CChasePlayer>()) {
        sf::VertexArray lines(sf::LinesStrip, 2);
        lines[0].position.x = e->get<CTransform>().pos.x;
        lines[0].position.y = e->get<CTransform>().pos.y;
        lines[0].color = sf::Color::Black;
        lines[1].position.x = player()->get<CTransform>().pos.x;
        lines[1].position.y = player()->get<CTransform>().pos.y;
        lines[1].color = sf::Color::Black;
        game->getWindow().draw(lines);
        dot.setPosition(e->get<CChasePlayer>().home.x, e->get<CChasePlayer>().home.y);
        game->getWindow().draw(dot);
      }
    }
  }
  ImGui::SFML::Render(game->getWindow());
  game->getWindow().display();
}

void Scene_Zelda::sGUI() {
  // Not required but may help with debugging
  ImGui::Begin("Scene Properties");

  if (ImGui::BeginTabBar("MyTabBar")) {
    if (ImGui::BeginTabItem("Debug")) {
      ImGui::Checkbox("Draw Grid", &drawGrid);
      ImGui::Checkbox("Draw Textures", &drawTextures);
      ImGui::Checkbox("Draw Debug", &drawCollision);
      ImGui::Checkbox("Follow Cam", &follow);

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Animations")) {
      if (ImGui::CollapsingHeader("Animations", ImGuiTreeNodeFlags_None)) {
        ImGui::Indent();
        int count = 0;
        for (auto it = game->getAssets().getAnimationMap().begin(); it != game->getAssets().getAnimationMap().end();
             ++it) {
          count++;
          sf::Sprite sprite = ((Animation)it->second).getSprite();
          GLuint textureID = sprite.getTexture()->getNativeHandle();
          ImTextureID imguiTextureID = (void *)(intptr_t)textureID;
          ImGui::ImageButton(imguiTextureID, ImVec2(64.0, 64.0), ImVec2(0, 0),
                             ImVec2(1.0 / ((Animation)it->second).getFrameCount(), 1.0));
          if ((count % 6) != 0) {
            ImGui::SameLine();
          }
        }
        ImGui::Unindent();
      }

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

void Scene_Zelda::sPausedGUI() {
    // resume, save, main menu, and quit
    if (paused) {
        // Center the window on the screen
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 windowSize = ImVec2(300, 200);
        ImVec2 windowPos = ImVec2((screenSize.x - windowSize.x) / 2, (screenSize.y - windowSize.y) / 2);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);

        ImGui::Begin("Pause Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("Paused");
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 20));

        float windowWidth = ImGui::GetWindowWidth();
        float buttonWidth = 200.0f;
        // Resume
        ImGui::SetCursorPosX((windowWidth - buttonWidth) / 2.0f);
        if (ImGui::Button("Resume", ImVec2(buttonWidth, 0))) {
            paused = false;
        }
        // Save
        ImGui::SetCursorPosX((windowWidth - buttonWidth) / 2.0f);
        if (ImGui::Button("Save", ImVec2(buttonWidth, 0))) {
            saveLevel();
        }
        // Main Menu
        ImGui::SetCursorPosX((windowWidth - buttonWidth) / 2.0f);
        if (ImGui::Button("Return to Main Menu", ImVec2(buttonWidth, 0))) {
            onEnd();
        }
        // Quit
        ImGui::SetCursorPosX((windowWidth - buttonWidth) / 2.0f);
        if (ImGui::Button("Quit", ImVec2(buttonWidth, 0))) {
            game->quit();
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }
}

void Scene_Zelda::drawLine(Vec2f p1, Vec2f p2) {
  sf::Vertex line[] = {sf::Vertex(sf::Vector2f(p1.x, p1.y)), sf::Vertex(sf::Vector2f(p2.x, p2.y))};

  game->getWindow().draw(line, 2, sf::Lines);
}

Entity *Scene_Zelda::player() {
  auto &players = entityManager.getEntities("player");

  return players[0];
}

int Scene_Zelda::width() { return game->getWindow().getSize().x; }

int Scene_Zelda::height() { return game->getWindow().getSize().y; }
