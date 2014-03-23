/*
-----------------------------------------------------------------------------
Filename:    TileGame.h
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
 */
#ifndef __TileGame_h_
#define __TileGame_h_

#include "BaseGame.h"
#include "BallManager.h"
#include "SoundManager.h"
#include "NetManager.h"

#include <vector>
#include <string>

const static int WALL_SIZE = 2400;
const static int PLANE_DIST = WALL_SIZE / 2;                        // the initial offset from the center.
const static int NUM_TILES_ROW = 5;                                 // number of tiles in each row of a wall.
const static int NUM_TILES_WALL = NUM_TILES_ROW * NUM_TILES_ROW;    // number of total tiles on a wall.
const static int TILE_WIDTH = WALL_SIZE / NUM_TILES_ROW;
const static int SWEEP_MS = 150;
const static int BROAD_MS = 8000;

int ticks = 0;

const Ogre::Quaternion RING_FLIP(Ogre::Degree(90), Ogre::Vector3::UNIT_X);

struct PlayerData {
  Uint32 host;
  Ogre::Quaternion newDir;
  Ogre::Vector3 newPos;
  Ogre::Vector3 shotDir;
  Ogre::Vector3 velocity;
  double shotForce;
};

struct PlayerOldData {
  Ogre::Quaternion oldDir;
  Ogre::Vector3 oldPos;
  double delta;
};

/* Since we should not have the physics sim running on clients, the server needs
 * to keep track of and distribute all ball locations and velocities. This
 * struct theoretically can handle up to 27 balls at one time by using 64 bits
 * per ball and keeping track of how many balls/64-bit fields should be read.
 *
 * Per ball:
 *  11 bits - x position interpreted as integer (max 2048)
 *  11 bits - y position interpreted as integer (max 2048)
 *  11 bits - z position interpreted as integer (max 2048)
 *  10 bits - x velocity interpreted as integer (max 1024)
 *  10 bits - y velocity interpreted as integer (max 1024)
 *  10 bits - z velocity interpreted as integer (max 1024)
 *   1 bit  - padding
 *
 * This is very low-level and slightly hacky, but it could allow us to update
 * all balls with only 220 bytes every single network update. It is possible
 * to update all balls with full Vector3s, but it would require 652 bytes per
 * update (for 27 balls).
 *
 * The current network buffer size is 128 bytes. This can change but must be
 * lower than the MTU (maximum transmission unit) set by the hardware and the
 * network. This is commonly 1500 bytes but can be as low as 500. Size also
 * slows down the transmission. We can discuss and/or test.
 */
struct BallData {
  int numBalls;                     //   4 bytes
  Uint64 posAndVel[27];             // 216 bytes

  // Alternate
  // Ogre::Vector3 posAndVel[54];   // 648 bytes
};

class TileGame : public BaseGame
{
public:
  TileGame(void);
  virtual ~TileGame(void);

  Ogre::RenderWindow * getWindow(void) { return mWindow; }
  Ogre::Timer * getTimer(void) { return mTimer; }
  OIS::Mouse * getMouse(void) { return mMouse; }
  OIS::Keyboard * getKeyboard(void) { return mKeyboard; }

protected:
  virtual bool configure(void);
  virtual void createCamera(void);
  virtual void createScene(void);
  virtual void createFrameListener(void);
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
  virtual bool keyPressed( const OIS::KeyEvent &arg );
  //virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  Ogre::Timer *mTimer, timer, *netTimer;
  Ogre::SceneNode* headNode;
  Ogre::Light* panelLight;
  Ogre::Vector3 mDirection;
  Ogre::Real mSpeed;

  std::deque<Ogre::Entity *> allTileEntities;
  std::deque<Ogre::SceneNode *> tileList;
  std::deque<Ogre::Entity *> tileEntities;
  std::deque<Ogre::SceneNode *> tileSceneNodes;
  std::vector<Ogre::Entity *> playerEntities;
  std::vector<Ogre::SceneNode *> playerNodes;
  std::vector<PlayerData *> playerData;
  std::vector<PlayerOldData *> playerOldData;

  OgreBites::ParamsPanel *scorePanel, *playersWaitingPanel;
  OgreBites::Label *congratsPanel, *chargePanel, *clientAcceptDescPanel,
  *clientAcceptOptPanel, *serverStartPanel;
  Ogre::Overlay* crosshairOverlay;

  TileSimulator *sim;
  BallManager *ballMgr;
  SoundManager *soundMgr;
  NetManager *netMgr;

  SoundFile boing, gong, music;
  SoundFile chirp;
  std::vector<SoundFile> noteSequence;
  int noteIndex;
  bool paused, gameStart, gameDone, animDone, isCharging, connected, server,
  netActive, invitePending, inviteAccepted, multiplayerStarted;
  int score, shotsFired, currLevel, currTile, winTimer, tileCounter, chargeShot,
  nPlayers;
  double slowdownval;
  std::string invite;
  int ballsounddelay;


  void shootBall(int idx, int x, int y, int z, double force) {
    Ogre::Vector3 direction = playerData[idx]->shotDir;
    Ogre::SceneNode* nodepc = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Ogre::Entity* ballMeshpc = mSceneMgr->createEntity("sphere.mesh");

    if (ballMgr->isPlayerBall(idx))
      ballMgr->removePlayerBall(idx);

    ballMeshpc->setCastShadows(true);
    nodepc->attachObject(ballMeshpc);
    ballMgr->setPlayerBall(ballMgr->addBall(nodepc, x, y, z, 100), idx);
    ballMgr->playerBalls[idx]->applyForce(force, direction);
  }

  void ballSetup (int cubeSize) {
    float ballSize = 200;                   //diameter
    float meshSize =  ballSize / 200;       //200 is size of the mesh.

    for (int x = 0; x < cubeSize; x++) {
      for (int y = 0; y < cubeSize; y++) {
        for (int z = 0; z < cubeSize; z++) {
          Ogre::Entity* ballMesh = mSceneMgr->createEntity("sphere.mesh");
          ballMesh->setMaterialName("Examples/SphereMappedRustySteel");
          ballMesh->setCastShadows(true);

          // Attach the node.
          Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
          headNode->attachObject(ballMesh);
          headNode->setScale(Ogre::Vector3(meshSize, meshSize, meshSize));
          ballMgr->addMainBall(headNode, x * ballSize, y * ballSize, z * ballSize, ballSize/2);
        }
      }
    }
  }

  void levelSetup(int num) {
    if(num > 50)
      num = 50;
    if (!connected)
      srand(time(0));
    else
      srand(1);

    Ogre::Plane wallTile = Ogre::Plane(Ogre::Vector3::UNIT_X, -PLANE_DIST +1);

    // Since each mesh starts at the center of the plane, we need to offset it
    // to the top right corner of the plane and start counting from there.
    int offset = WALL_SIZE/2 - TILE_WIDTH/2;

    int x, y, z;
    x = y = z = 0;

    std::vector<int> randomnumbers;
    for(int i = 0; i < 50; i++)
      randomnumbers.push_back(i);

    for(int i = 0; i < num; i++) {
      std::stringstream ss;
      std::stringstream ssDebug;
      ss << (i + tileCounter);

      int rn = std::rand() % randomnumbers.size(); // get random tile in list of unused tiles
      int tileNum = randomnumbers[rn];
      randomnumbers.erase(randomnumbers.begin() + rn);
      ssDebug << tileNum;
      // std::cout << "Random number1: " + ssDebug.str() << std::endl;
      ssDebug.str(std::string());

      int wallTileNum = tileNum % NUM_TILES_WALL; //possible number of tiles per wall.
      ssDebug << wallTileNum;
      // std::cout << "Random number: " + ssDebug.str() << std::endl;
      int row = wallTileNum / NUM_TILES_ROW; //5 is the number of tiles per row.
      int col = wallTileNum % NUM_TILES_ROW;

      ssDebug.str(std::string());
      ssDebug << row;
      ssDebug << " ";
      ssDebug << col;

      // std::cout << "Row/col " + ssDebug.str() << std::endl;

      Ogre::SceneNode* node1; //= mSceneMgr->getRootSceneNode()->createChildSceneNode();
      int xsize = 240;
      int ysize = 240;
      int zsize = 240;

      // left
      if(tileNum < 25) {
        // set up x y z units of 0, 1 or -1, which will be used when we setPosition
        // set up our WallTileLeft or whatever to point to the right direction.
        wallTile = Ogre::Plane(Ogre::Vector3::UNIT_X, 1);
        x = 0;
        y = -1 * (row * TILE_WIDTH) + offset;
        z = -1 * (col * TILE_WIDTH) + offset;
        xsize = 10;
        node1 = mSceneMgr->getSceneNode("leftNode")->createChildSceneNode();
      }

      // front
      else if (tileNum < 50) {
        x = 1 * (col * TILE_WIDTH) - offset;
        y = -1 * (row * TILE_WIDTH) + offset;
        z = 0;
        zsize = 10;
        wallTile = Ogre::Plane(Ogre::Vector3::UNIT_Z, 1);
        node1 = mSceneMgr->getSceneNode("frontNode")->createChildSceneNode();
      }

      // right
      else if (tileNum < 75) {
        x = 0;
        y = -1 * (row * TILE_WIDTH) + offset;
        z = 1 * (col * TILE_WIDTH) - offset;
        xsize = 10;
        wallTile = Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_X, 1);
        node1 = mSceneMgr->getSceneNode("rightNode")->createChildSceneNode();
      }

      // back
      else if (tileNum < 100) {
        x = 1 * (col * TILE_WIDTH) - offset;
        y = -1 * (row * TILE_WIDTH) + offset;
        z = 0;
        zsize = 10;
        wallTile = Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Z, 1);
        node1 = mSceneMgr->getSceneNode("backNode")->createChildSceneNode();
      }

      // Build the entity name based on which tile number it is.
      std::string str = "tile";
      str.append(ss.str());
      std::string entityStr = "tileEntity";
      entityStr.append(ss.str());
      // std::cout << "tileEntityName: " + entityStr << std::endl;

      Ogre::MeshManager::getSingleton().createPlane(str,
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallTile,
          TILE_WIDTH, TILE_WIDTH, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
      Ogre::Entity* tile = mSceneMgr->createEntity(entityStr, str);

      node1->translate(x ,y, z); //1600 / 5 is our tilewidth
      node1->attachObject(tile);
      tile->setMaterialName("Examples/Chrome");
      tile->setCastShadows(false);
      sim->addTile(node1, xsize, ysize, zsize);
      tileEntities.push_back(tile);
      allTileEntities.push_back(tile);
      tileList.push_back(node1);
      tileSceneNodes.push_back(node1);
    }
    tileCounter += num;

    int it, numballs;
    it = numballs = 1;
    while (numballs < num) {
      it++;
      numballs = it * it * it;
    }

    ballSetup(it);

    soundMgr->playSound(gong);

    gameDone = animDone = false;
    currTile = tileEntities.size() - 1;
    timer.reset();
  }


  void levelTearDown() {
    ballMgr->clearBalls();

    for(int i = 0; i < tileList.size(); i++)
      mSceneMgr->destroySceneNode(tileList[i]);
    for(int i = 0; i < allTileEntities.size(); i++)
      mSceneMgr->destroyEntity(allTileEntities[i]);
    tileList.clear();
    allTileEntities.clear();

    currLevel++;
  }

  void setLevel(int num) {
    levelTearDown();
    currLevel = num;
    score = 0;
    shotsFired = 0;
    levelSetup(num);
  }

  void drawPlayers() {
    Ogre::SceneNode *ringNode;
    Ogre::Entity *ringEnt;
    std::ostringstream playerName;
    int i;

    for (i = 0; i < playerData.size(); i++) {
      playerName << playerData[i]->host;
      ringNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(playerName.str());
      ringEnt = mSceneMgr->createEntity("torus.mesh");
      ringNode->attachObject(ringEnt);
      ringNode->rotate(RING_FLIP);
      ringNode->setOrientation(playerData[i]->newDir);
      ringNode->setScale(100, 100, 100);
      ringNode->setPosition(playerData[i]->newPos);

      playerNodes.push_back(ringNode);
      playerEntities.push_back(ringEnt);
    }
  }

  void movePlayers() {
    std::ostringstream playerName;
    Ogre::Vector3 oldPos, newPos, drawPos;
    Ogre::Quaternion newDir, oldDir, drawDir;
    Ogre::Vector3 velocity;
    Ogre::SceneNode *node;
    int i;

    for (i = 0; i < nPlayers; i++) {
      double delta;

      // Update position.
      newPos = playerData[i]->newPos;
      oldPos = playerOldData[i]->oldPos;
      delta = playerOldData[i]->delta;
      playerOldData[i]->delta += 1;

      drawPos = newPos;
      drawPos += playerData[i]->velocity * delta / 60.0;

      oldDir = playerOldData[i]->oldDir;
      newDir = playerData[i]->newDir;
      // drawDir = newDir + (newDir - oldDir) * (delta / 10.0);
      drawDir = Ogre::Quaternion::Slerp(delta * 0.1f, oldDir, newDir);

      playerName << playerData[i]->host;
      node = mSceneMgr->getSceneNode(playerName.str());

      node->setOrientation(drawDir);
      node->pitch(Ogre::Degree(90));
      node->setPosition(drawPos);
      //node->translate(delta);
    }
  }

  void updatePlayers(double force = 0, Ogre::Vector3 dir = Ogre::Vector3::ZERO) {
    PlayerData single;
    int i, pdSize, tagSize;

    pdSize = sizeof(PlayerData);
    tagSize = sizeof(Uint32);

    // Self
    single.host = netMgr->getIPnbo();
    single.newPos = mCamera->getPosition();
    single.newDir = mCamera->getOrientation();
    single.shotForce = force;
    single.shotDir = dir;
    single.velocity = mCameraMan->getVelocity();
    memcpy(netMgr->udpServerData[nPlayers].input, &UINT_UPDPL, tagSize);
    memcpy((netMgr->udpServerData[nPlayers].input + 4), &single, pdSize);
    netMgr->udpServerData[nPlayers].updated = true;

    // Clients
    for (i = 0; i < playerData.size(); i++) {
      memcpy(netMgr->udpServerData[i].input, &UINT_UPDPL, tagSize);
      memcpy((netMgr->udpServerData[i].input + 4), playerData[i], pdSize);
      netMgr->udpServerData[i].updated = true;
    }

    netMgr->messageClients(PROTOCOL_UDP);
  }

  void updateServer(double force = 0, Ogre::Vector3 dir = Ogre::Vector3::ZERO) {
    PlayerData single;
    int pdSize, tagSize;

    pdSize = sizeof(PlayerData);
    tagSize = sizeof(Uint32);

    // Self
    single.host = netMgr->getIPnbo();
    single.newPos = mCamera->getPosition();
    single.newDir = mCamera->getOrientation();
    single.shotForce = force;
    single.shotDir = dir;
    single.velocity = mCameraMan->getVelocity();

    if (force) {
      memcpy(netMgr->tcpServerData.input, &UINT_BLSHT, tagSize);
      memcpy((netMgr->tcpServerData.input + 4), &single, pdSize);
      netMgr->tcpServerData.updated = true;
      netMgr->messageServer(PROTOCOL_TCP);
    } else {
      memcpy(netMgr->udpServerData[0].input, &UINT_UPDSV, tagSize);
      memcpy((netMgr->udpServerData[0].input + 4), &single, pdSize);
      netMgr->udpServerData[0].updated = true;
      netMgr->messageServer(PROTOCOL_UDP);
    }
  }

  void startMultiplayer() {
    tileEntities.clear();
    tileSceneNodes.clear();
    sim->clearTiles();
    gameDone = true;

    setLevel(1);
    drawPlayers();
    ballMgr->initMultiplayer(nPlayers);

    multiplayerStarted = true;
  }

  void addPlayer(Uint32 *data) {
    PlayerData *newPlayer = new PlayerData;
    PlayerOldData *newOldPlayer = new PlayerOldData;

    memcpy(newPlayer, data, sizeof(PlayerData));

    newOldPlayer->oldPos = newPlayer->newPos;
    newOldPlayer->oldDir = newPlayer->newDir;
    newOldPlayer->delta = 0;

    playerData.push_back(newPlayer);
    playerOldData.push_back(newOldPlayer);
  }

  void modifyPlayer(int j, Uint32 *data) {
    playerOldData[j]->oldPos = playerData[j]->newPos;
    playerOldData[j]->oldDir = playerData[j]->newDir;
    playerOldData[j]->delta = 0;

    memcpy(playerData[j], data, sizeof(PlayerData));

    // Did they launch a ball?  Trigger now before buffer overwritten!
    if (playerData[j]->shotForce) {
      std::cout << "Shot fired." << std::endl;
      Ogre::Vector3 newPos = playerData[j]->newPos;
      shootBall(j, newPos.x, newPos.y, newPos.z, playerData[j]->shotForce);
      playerData[j]->shotForce = 0;
    }
  }

  void notifyPlayers() {
    PlayerData single;
    int i, pdSize, tagSize;

    pdSize = sizeof(PlayerData);
    tagSize = sizeof(Uint32);

    // Self
    single.host = netMgr->getIPnbo();
    single.newPos = mCamera->getPosition();
    single.newDir = mCamera->getOrientation();
    single.shotForce = 0;
    single.shotDir = Ogre::Vector3::ZERO;
    memcpy(netMgr->udpServerData[nPlayers].input, &UINT_ADDPL, tagSize);
    memcpy((netMgr->udpServerData[nPlayers].input + 4), &single, pdSize);
    netMgr->udpServerData[nPlayers].updated = true;

    // Clients
    for (i = 0; i < playerData.size(); i++) {
      memcpy(netMgr->udpServerData[i].input, &UINT_ADDPL, tagSize);
      memcpy((netMgr->udpServerData[i].input + 4), playerData[i], pdSize);
      netMgr->udpServerData[i].updated = true;
    }

    netMgr->messageClients(PROTOCOL_UDP);
  }

  void notifyServer() {
    PlayerData single;
    int pdSize, tagSize;

    pdSize = sizeof(PlayerData);
    tagSize = sizeof(Uint32);

    // Self
    single.host = netMgr->getIPnbo();
    single.newPos = mCamera->getPosition();
    single.newDir = mCamera->getOrientation();
    single.shotForce = 0;
    single.shotDir = Ogre::Vector3::ZERO;
    memcpy(netMgr->udpServerData[0].input, &UINT_ADDPL, tagSize);
    memcpy((netMgr->udpServerData[0].input + 4), &single, pdSize);
    netMgr->udpServerData[0].updated = true;
    netMgr->messageServer(PROTOCOL_UDP);
  }

  void simonSaysAnim() {
    if(gameDone) {
      currTile = -2;

      if(panelLight != NULL) {
        mSceneMgr->destroyLight(panelLight);
        panelLight = NULL;
      }
      return;
    }

    long currTime = timer.getMilliseconds();
    int numTiles = tileEntities.size();
    int startTime = 0;          // starts 2 secs into the game.
    int timePerTile = 1500;     // each tile lights up for this duration (2 secs)
    int waitTime = 100;         // waits 500ms between each tile being lit up.

    int animStart = (waitTime + timePerTile) * ((tileEntities.size() - 1) - currTile) + startTime;
    int animEnd = animStart + timePerTile;

    if(currTile >= -1) {
      if(currTime > animStart && currTime <= animEnd) {
        if(currTile < noteSequence.size() && currTile >= 0) {
          //soundMgr->playSound(noteSequence[currTile]);

          soundMgr->playSound(noteSequence[currTile], tileSceneNodes[currTile]->_getDerivedPosition() , mCamera);

        }
        // Revert previous tile to original texture
        if(currTile + 1 < tileEntities.size() && currTile >= -1) {
          tileEntities[currTile + 1]->setMaterialName("Examples/Chrome");
        }

        if(currTile >= 0) {
          tileEntities[currTile]->setMaterialName("Examples/SpaceSky");

          if(panelLight != NULL)
            mSceneMgr->destroyLight(panelLight);

          // Create a light
          panelLight = mSceneMgr->createLight("Panel");
          panelLight->setCastShadows(false);
          panelLight->setType(Ogre::Light::LT_SPOTLIGHT);
          int x = tileSceneNodes[currTile]->_getDerivedPosition().x;
          int y = tileSceneNodes[currTile]->_getDerivedPosition().y;
          int z = tileSceneNodes[currTile]->_getDerivedPosition().z;

          if (x < 0)
            x += 10;
          else if (x > 0)
            x -= 10;

          if (z < 0)
            z += 10;
          else if (z > 0)
            z -= 10;

          // std::cout << "x: " << x << " y: " << y << " z: " << z << "\n";
          panelLight->setDiffuseColour(0.70, 0.50, 0.30);
          panelLight->setDirection(x, y, z);
          panelLight->setPosition(0, 0, 0);
          panelLight->setSpotlightFalloff(0);
          panelLight->setAttenuation(4000, 0.0, 0.0001, 0.0000005);

        } else {
          mSceneMgr->destroyLight(panelLight);
          panelLight = NULL;
        }

        // moves on to the next tile.
        currTile--;
        //std::cout << "c: " << currTile << "\n";
      }
    }
    else if (!animDone) {
      if (tileEntities.size() > 0) {
        tileEntities[0]->setMaterialName("Examples/Chrome");
        animDone = true;
      }
    }
  }
};

#endif // #ifndef __TileGame_h_
