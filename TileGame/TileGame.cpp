/*
-----------------------------------------------------------------------------
Filename:    TileGame.cpp
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
#include "TileGame.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

//-------------------------------------------------------------------------------------
TileGame::TileGame(void) :
mDirection(Ogre::Vector3::ZERO),
headNode(0),
ballMgr(0),
soundMgr(0),
netMgr(0),
sim(0),
panelLight(0),
scorePanel(0),
congratsPanel(0),
chargePanel(0),
clientAcceptDescPanel(0),
clientAcceptOptPanel(0),
serverStartPanel(0),
playersWaitingPanel(0),
crosshairOverlay(0),
boing(0),
music(0),
gong(0)
{
  gameDone = animDone = isCharging = paused = connected = server = netActive =
      invitePending = inviteAccepted = false;
  gameStart = true;

  mSpeed = score = shotsFired = tileCounter = winTimer = chargeShot =
      slowdownval = currTile = nPlayers = 0;
  currLevel = 1;

  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
}
//-------------------------------------------------------------------------------------
TileGame::~TileGame(void) {
  delete soundMgr;
  delete ballMgr;
  delete netMgr;
  delete sim;
}
//-------------------------------------------------------------------------------------
bool TileGame::configure() {
  bool ret = BaseGame::configure();

  // Sound //
  soundMgr = new SoundManager();
  soundMgr->initSoundManager();

  boing = soundMgr->loadSound("hit.wav");
  gong = soundMgr->loadSound("gong.wav");
  music = soundMgr->loadMusic("ambient.wav");

  soundMgr->setVolume(.5);
  soundMgr->playMusic();

  // Physics //
  sim = new TileSimulator();
  sim->initSimulator();

  // Balls //
  ballMgr = new BallManager(sim);
  ballMgr->initBallManager();

  // Networking //
  netMgr = new NetManager();
  netMgr->initNetManager();
  netMgr->addNetworkInfo(PROTOCOL_UDP);
  netActive = netMgr->startServer();

  return ret;
}
//-------------------------------------------------------------------------------------
void TileGame::createCamera(void) {
  BaseGame::createCamera();
  mCameraMan = new CameraMan(mCamera);
}
//-------------------------------------------------------------------------------------
void TileGame::createScene(void) {
  boxBound = Ogre::PlaneBoundedVolume(Ogre::Plane::NEGATIVE_SIDE);
  boxBound.planes.push_back(wallBack = Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Z, 0));
  boxBound.planes.push_back(wallFront = Ogre::Plane(Ogre::Vector3::UNIT_Z,0));
  boxBound.planes.push_back(wallDown = Ogre::Plane(Ogre::Vector3::UNIT_Y,0));
  boxBound.planes.push_back(wallUp = Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Y,0));
  boxBound.planes.push_back(wallLeft = Ogre::Plane(Ogre::Vector3::UNIT_X, 0));
  boxBound.planes.push_back(wallRight = Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_X,0));

  // Use the planes from above to generate new meshes for walls.
  Ogre::MeshManager::getSingleton().createPlane("ground",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallDown,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);
  Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
  entGround->setMaterialName("Custom/texture_blend");
  entGround->setCastShadows(false);
  Ogre::SceneNode* nodeGround = mSceneMgr->getRootSceneNode()->createChildSceneNode();
  nodeGround->setPosition(0 , -PLANE_DIST, 0);
  nodeGround->attachObject(entGround);

  Ogre::MeshManager::getSingleton().createPlane("ceiling",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallUp,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, 2, 2, Ogre::Vector3::UNIT_Z);
  Ogre::Entity* entCeiling = mSceneMgr->createEntity("CeilingEntity", "ceiling");
  entCeiling->setMaterialName("Examples/CloudySky");
  entCeiling->setCastShadows(false);
  Ogre::SceneNode* nodeCeiling = mSceneMgr->getRootSceneNode()->createChildSceneNode();
  nodeCeiling->setPosition(0 , PLANE_DIST, 0);
  nodeCeiling->attachObject(entCeiling);

  Ogre::MeshManager::getSingleton().createPlane("back",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallBack,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entBack = mSceneMgr->createEntity("BackEntity", "back");
  entBack->setMaterialName("Examples/Rockwall");
  entBack->setCastShadows(false);
  Ogre::SceneNode* nodeBack = mSceneMgr->getRootSceneNode()->createChildSceneNode("backNode");
  nodeBack->setPosition(0 , 0, PLANE_DIST);
  nodeBack->attachObject(entBack);

  Ogre::MeshManager::getSingleton().createPlane("front",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallFront,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entFront = mSceneMgr->createEntity("FrontEntity", "front");
  entFront->setMaterialName("Examples/Rockwall");
  entFront->setCastShadows(false);
  Ogre::SceneNode* nodeFront = mSceneMgr->getRootSceneNode()->createChildSceneNode("frontNode");
  nodeFront->setPosition(0 , 0, -PLANE_DIST);
  nodeFront->attachObject(entFront);

  Ogre::MeshManager::getSingleton().createPlane("left",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallLeft,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entLeft = mSceneMgr->createEntity("LeftEntity", "left");
  entLeft->setMaterialName("Examples/Rockwall");
  entLeft->setCastShadows(false);
  Ogre::SceneNode* nodeLeft = mSceneMgr->getRootSceneNode()->createChildSceneNode("leftNode");
  nodeLeft->setPosition(-PLANE_DIST , 0, 0);
  nodeLeft->attachObject(entLeft);

  Ogre::MeshManager::getSingleton().createPlane("right",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallRight,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entRight = mSceneMgr->createEntity("RightEntity", "right");
  entRight->setMaterialName("Examples/Rockwall");
  entRight->setCastShadows(false);
  Ogre::SceneNode* nodeRight = mSceneMgr->getRootSceneNode()->createChildSceneNode("rightNode");
  nodeRight->setPosition(PLANE_DIST, 0, 0);
  nodeRight->attachObject(entRight);

  // Set ambient light
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.35, 0.35, 0.35));

  // Create a light
  Ogre::Light* lSun = mSceneMgr->createLight("SunLight");
  lSun->setType(Ogre::Light::LT_POINT);
  lSun->setDiffuseColour(0.95, 0.95, 1.00);
  lSun->setPosition(0,1400,0);
  lSun->setAttenuation(3250, 1.0, 0.0000000001, 0.000001);

  sim->createBounds(PLANE_DIST);

  levelSetup(currLevel);
}
//-------------------------------------------------------------------------------------
void TileGame::createFrameListener(void) {
  BaseGame::createFrameListener();

  Ogre::StringVector scorelist;
  scorelist.push_back("Score");
  scorelist.push_back("Shots Fired");
  scorelist.push_back("Current Level");
  Ogre::StringVector playerCountTag;
  playerCountTag.push_back("Current Players:");

  scorePanel = mTrayMgr->createParamsPanel(OgreBites::TL_TOPLEFT,
      "ScorePanel", 200, scorelist);
  congratsPanel = mTrayMgr->createLabel(OgreBites::TL_TOP,
      "CongratsPanel", "NULL", 300);
  chargePanel = mTrayMgr->createLabel(OgreBites::TL_BOTTOM,
      "Chargepanel", "|", 300);
  serverStartPanel = mTrayMgr->createLabel(OgreBites::TL_TOPRIGHT,
      "ServerStartPanel", "Press (B) to start when ready.", 300);
  clientAcceptDescPanel = mTrayMgr->createLabel(OgreBites::TL_TOPRIGHT,
      "ClientAcceptDescPanel", "Join multiplayer game?", 200);
  clientAcceptOptPanel = mTrayMgr->createLabel(OgreBites::TL_TOPRIGHT,
      "ClientAcceptOptPanel", "(Y)es or (N)o", 150);
  playersWaitingPanel = mTrayMgr->createParamsPanel(OgreBites::TL_BOTTOMRIGHT,
      "PlayersWaitingPanel", 200, playerCountTag);

  congratsPanel->hide();
  serverStartPanel->hide();
  clientAcceptDescPanel->hide();
  clientAcceptOptPanel->hide();
  // playersWaitingPanel->hide();

  Ogre::OverlayManager& overlayMgr = Ogre::OverlayManager::getSingleton();
  crosshairOverlay = overlayMgr.create("Crosshair");

  Ogre::OverlayContainer* panel = static_cast<Ogre::OverlayContainer*>(
      overlayMgr.createOverlayElement("Panel", "PanelName"));
  panel->setPosition(0.488, 0.475);
  panel->setDimensions(0.025, 0.0375);
  panel->setMaterialName("Examples/Crosshair");
  crosshairOverlay->add2D(panel);
  crosshairOverlay->show();
}
//-------------------------------------------------------------------------------------
bool TileGame::frameRenderingQueued(const Ogre::FrameEvent& evt) {
  bool ret = BaseGame::frameRenderingQueued(evt);

  int i;

  if (paused)
    slowdownval += 1/1800.f;
  else {
    bool hit = sim->simulateStep(slowdownval);

    if (hit && !gameDone) {
      soundMgr->playSound(boing);
      score++;

      if (!tileEntities.empty()) {
        tileEntities.back()->setMaterialName("Examples/BumpyMetal");
        tileEntities.pop_back();
        tileSceneNodes.pop_back();
      }

      if (tileEntities.empty()) {
        gameDone = true;
        winTimer = 0;
        congratsPanel->show();
        ballMgr->enableGravity();
      }
    }
  }

  if (gameDone && !paused && winTimer++ > 320) {
    levelTearDown();
    levelSetup(currLevel);
    congratsPanel->hide();
  }

  if (!animDone)
    simonSaysAnim();

  if (isCharging && chargeShot < 10000)
    chargeShot += 70;

  /* Update SDKTrays panels. */
  if (!mTrayMgr->isDialogVisible()) {
    // ScorePanel
    scorePanel->setParamValue(0, Ogre::StringConverter::toString(score));
    scorePanel->setParamValue(1, Ogre::StringConverter::toString(shotsFired));
    scorePanel->setParamValue(2, Ogre::StringConverter::toString(currLevel));

    // Level complete notification.
    std::stringstream grats;
    grats << "Moving to level ";
    grats << (currLevel + 1);
    grats << "...";
    congratsPanel->setCaption(grats.str());

    // Charge shot meter.
    std::stringstream scharge;
    scharge << "|";
    for(int i = 1000; i < chargeShot; i += 160)
      scharge << "|";
    chargePanel->setCaption(scharge.str());

    // Number of players in the game.
    playersWaitingPanel->setParamValue(0,
        Ogre::StringConverter::toString(nPlayers + 1));
  }

  /* ***********************************************************************
   * Multiplayer Code
   */
  int limiter = 0;
  if (netActive) {
    if (!(limiter % 100) && netMgr->scanForActivity()) {
      std::string cmd, cmdArgs;
      /*  Received an update!  */

      if (!server) {
        if (!connected) {                       /* Running as single player. */
          if (!invitePending) {
            // Accept only the first invitation received if spammed.
            invite = std::string(netMgr->udpServerData.output);
            if (std::string::npos != invite.find(STR_OPEN)) {
              clientAcceptDescPanel->show();
              clientAcceptOptPanel->show();
              invitePending = true;
            }
          } else {
            // Invited! Do we want to join the server in a multiplayer instance?
            if (inviteAccepted) {
              connected = netMgr->joinMultiPlayer(invite);
              if (!connected) {
                std::cout << "TileGame: Failed to join server." << std::endl;
                netMgr->close();
                netActive = false;
              }
            }
          }
        } else {              /* Currently connected to a game as a client. */
          // Process UDP messages.
          if (netMgr->udpServerData.updated) {
            cmd = std::string(netMgr->udpServerData.output);
            if (0 == cmd.compare(STR_BEGIN)) {
              startMultiplayer();
            }
            if (-1 < cmd.find(STR_ADDPL)) {
              cmdArgs = cmd.substr(STR_ADDPL.length());
              std::cout << cmdArgs << std::endl;
              // PlayerData *newPlayer = new PlayerData;
              // memcpy(newPlayer, cmdArgs.c_str(), cmdArgs.length());
            }

            netMgr->udpServerData.updated = false;
          }
          // Process TCP messages.
          if (netMgr->tcpServerData.updated) {
            cmd = std::string(netMgr->tcpServerData.output);
            if (0 == cmd.compare(STR_BEGIN)) {
              startMultiplayer();
            } else if (-1 < cmd.find(STR_ADDPL)) {
              cmdArgs = cmd.substr(STR_ADDPL.length());
              std::cout << cmdArgs << std::endl;
              // PlayerData *newPlayer = new PlayerData;
              // memcpy(newPlayer, cmdArgs.c_str(), cmdArgs.length());
            }

            netMgr->tcpServerData.updated = false;
          }
        }
      } else {
        if (!connected) {        /* Initiated a server, but no game started. */
          nPlayers = netMgr->getClients();
        } else {                    /* Currently hosting a game as a server. */
          // Process UDP messages.
          if (netMgr->udpServerData.updated) {
            cmd = std::string(netMgr->udpServerData.output);
            // Compare cmd to command tag or use raw data.

            netMgr->udpServerData.updated = false;
          }
          // Process TCP messages.
          if (netMgr->tcpServerData.updated) {
            cmd = std::string(netMgr->tcpServerData.output);
            // Compare cmd to command tag or use raw data.

            netMgr->tcpServerData.updated = false;
          }
        }
      }
    } else {                                       /*  No updates received.  */
      // What do we do?
    }

    /*
     * Message clients with global positions.
     */


    /*
     * Update clients' positions locally.
     */
    for (i = 0; i < nPlayers; i++) {
      // Set position.
    }

    if (limiter++ > 10000)
      limiter = 0;
  }

  return ret;
}
//-------------------------------------------------------------------------------------
bool TileGame::keyPressed( const OIS::KeyEvent &arg ) {
  if (arg.key == OIS::KC_ESCAPE) {
    mShutDown = true;
  }
  else if (arg.key == OIS::KC_SPACE) {
  }
  else if (arg.key == OIS::KC_Y) {
    if (invitePending) {
      inviteAccepted = true;

      clientAcceptDescPanel->hide();
      clientAcceptOptPanel->hide();
      invitePending = false;
    }
  }
  else if (arg.key == OIS::KC_N) {
    if (invitePending) {
      clientAcceptDescPanel->hide();
      clientAcceptOptPanel->hide();
      invitePending = false;
    }
  }
  else if (arg.key == OIS::KC_B) {
    if (server && !connected) {
      netMgr->messageClients(STR_BEGIN.c_str(), STR_BEGIN.length());
      netMgr->denyConnections();
      serverStartPanel->hide();
      connected = true;

      startMultiplayer();
    }
  }
  else if (arg.key == OIS::KC_P) {
    paused = !paused;
    slowdownval = 0.0;

    soundMgr->toggleSound();
  }
  else if (arg.key == OIS::KC_M) {
    soundMgr->toggleSound();
  }
  else if (arg.key == OIS::KC_O) {
    if (netActive) {
      if (!server) {
        server = netMgr->multiPlayerInit(24);
        if (server) {
          playersWaitingPanel->show();
          serverStartPanel->show();
        }
      } else if (connected) {
        std::cout << "TileGame: Exiting multiplayer game. Resuming single"
            " player mode at current level." << std::endl;

        netMgr->close();
        netMgr->initNetManager();
        netMgr->addNetworkInfo(PROTOCOL_UDP);
        netActive = netMgr->startServer();

        server = connected = false;
        setLevel(currLevel);
      }
    }
  }
  else if (arg.key == OIS::KC_Q) {
    if(currTile >= -1)
      tileEntities[currTile+1]->setMaterialName("Examples/Chrome");
    currTile = tileEntities.size() - 1;
    timer.reset();
    animDone = false;
  }

  return BaseGame::keyPressed(arg);
}
//-------------------------------------------------------------------------------------
bool TileGame::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  isCharging = true;
  chargeShot = 0;

  return BaseGame::mousePressed(arg, id);
}
//-------------------------------------------------------------------------------------
bool TileGame::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  isCharging = false;
  if(chargeShot >= 1000 && !gameDone) {
    Ogre::Vector3 direction = mCamera->getOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z;
    Ogre::SceneNode* nodepc = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Ogre::Entity* ballMeshpc = mSceneMgr->createEntity("sphere.mesh");
    // std::cout << chargeShot << std::endl;
    double force = chargeShot * 0.8f;
    chargeShot = 0;

    if(ballMgr->isGlobalBall())
      ballMgr->removeGlobalBall();

    int x = mCamera->getPosition().x;
    int y = mCamera->getPosition().y;
    int z = mCamera->getPosition().z;

    ballMeshpc->setCastShadows(true);
    nodepc->attachObject(ballMeshpc);
    ballMgr->setGlobalBall(ballMgr->addBall(nodepc, x, y, z, 100));
    ballMgr->globalBall->applyForce(force, direction);
    shotsFired++;
  }

  return BaseGame::mouseReleased(arg, id);
}
//-------------------------------------------------------------------------------------


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
#if defined(OGRE_IS_IOS)
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
  int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
  [pool release];
  return retVal;
#elif (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

  mAppDelegate = [[AppDelegate alloc] init];
  [[NSApplication sharedApplication] setDelegate:mAppDelegate];
  int retVal = NSApplicationMain(argc, (const char **) argv);

  [pool release];

  return retVal;
#else
  // Create application object
  TileGame app;

  try {
    app.go();
  } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
    std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
#endif
  }
#endif
  return 0;
}

#ifdef __cplusplus
}
#endif