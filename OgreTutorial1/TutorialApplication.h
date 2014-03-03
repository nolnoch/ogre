/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.h
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
#ifndef __TutorialApplication_h_
#define __TutorialApplication_h_

#include "BaseApplication.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <vector>

const static int WALL_SIZE = 2400;
const static int PLANE_DIST = WALL_SIZE / 2;                        // the initial offset from the center.
const static int NUM_TILES_ROW = 5;                                 // number of tiles in each row of a wall.
const static int NUM_TILES_WALL = NUM_TILES_ROW * NUM_TILES_ROW;    // number of total tiles on a wall.
const static int TILE_WIDTH = WALL_SIZE / NUM_TILES_ROW;

class TutorialApplication : public BaseApplication
{
public:
  TutorialApplication(void);
  virtual ~TutorialApplication(void);

  Ogre::RenderWindow * getWindow(void) { return mWindow; }
  Ogre::Timer * getTimer(void) { return mTimer; }
  OIS::Mouse * getMouse(void) { return mMouse; }
  OIS::Keyboard * getKeyboard(void) { return mKeyboard; }

protected:
  virtual bool configure(void);
  virtual void createScene(void);
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  Ogre::Timer *mTimer, timer;
  Ogre::SceneNode* headNode;
  Ogre::Light* panelLight;
  Ogre::Vector3 mDirection;
  Ogre::Real mSpeed;
  Ogre::Sphere ballBound;
  Ogre::PlaneBoundedVolume boxBound;
  Ogre::Plane wallUp, wallDown, wallBack, wallFront, wallLeft, wallRight;

  std::deque<Ogre::Entity *> allTileEntities;
  std::deque<Ogre::SceneNode *> tileList;
  std::deque<Ogre::Entity *> tileEntities;
  std::deque<Ogre::SceneNode *> tileSceneNodes;

  OgreBites::ParamsPanel* scorePanel;
  OgreBites::Label* congratsPanel;
  OgreBites::Label* chargePanel;

  std::vector<Ball*> balls;
  Ball* globalBall;
  Simulator* sim;

  Ogre::Vector3 vZero;
  Mix_Chunk *boing, *gong;
  Mix_Music *music;
  bool sounding, paused, gameStart, gameDone, animDone, isCharging;
  int score, shotsFired, currLevel, currTile, winTimer, tileCounter, chargeShot;
  double slowdownval;


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
          Ball* ball = new Ball(headNode, x * ballSize, y * ballSize, z * ballSize, ballSize/2);
          sim->addMainBall(ball);
          balls.push_back(ball);
        }
      }
    }
  }

  void levelSetup(int num) {
    if(num > 50)
      num = 50;
    srand(time(0));

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
      std::cout << "Random number1: " + ssDebug.str() << std::endl;
      ssDebug.str(std::string());

      int wallTileNum = tileNum % NUM_TILES_WALL; //possible number of tiles per wall.
      ssDebug << wallTileNum;
      std::cout << "Random number: " + ssDebug.str() << std::endl;
      int row = wallTileNum / NUM_TILES_ROW; //5 is the number of tiles per row.
      int col = wallTileNum % NUM_TILES_ROW;

      ssDebug.str(std::string());
      ssDebug << row;
      ssDebug << " ";
      ssDebug << col;

      std::cout << "Row/col " + ssDebug.str() << std::endl;

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
      std::cout << "tileEntityName: " + entityStr << std::endl;

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

    if (sounding)
      Mix_PlayChannel(-1, gong, 0);

    gameDone = animDone = false;
    currTile = tileEntities.size() - 1;
    timer.reset();
  }


  void levelTearDown() {
    for(int i = 0; i < balls.size(); i++)
      sim->removeBall(balls[i]);
    balls.clear();

    for(int i = 0; i < tileList.size(); i++)
      mSceneMgr->destroySceneNode(tileList[i]);
    for(int i = 0; i < allTileEntities.size(); i++)
      mSceneMgr->destroyEntity(allTileEntities[i]);
    tileList.clear();
    allTileEntities.clear();

    currLevel++;
    sim->removeBall(globalBall);
    globalBall = NULL;
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

          std::cout << "x: " << x << " y: " << y << " z: " << z << "\n";
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
        std::cout << "c: " << currTile << "\n";
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

#endif // #ifndef __TutorialApplication_h_
