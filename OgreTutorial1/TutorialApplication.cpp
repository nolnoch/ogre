/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
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
#include "TutorialApplication.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

//-------------------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
: headNode(0),
  mSpeed(0),
  mDirection(Ogre::Vector3::ZERO),
  vZero(Ogre::Vector3::ZERO),
  boing(0),
  sounding(false),
  currLevel(1),
  score(0),
  shotsFired(0),
  tileCounter(0),
  globalBall(0),
  panelLight(0),
  gameStart(true),
  animDone(false)
{
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
}
//-------------------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
  Mix_CloseAudio();
  SDL_Quit();
}
//-------------------------------------------------------------------------------------
bool TutorialApplication::configure() {
  bool ret = BaseApplication::configure();

  // Initialize Audio [based on http://www.kekkai.org/roger/sdl/mixer/]
  /* We're going to be requesting certain things from our audio
           device, so we set them up beforehand */
  int audio_rate = 22050;
  Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
  int audio_channels = 2;
  int audio_buffers = 4096;

  /* This is where we open up our audio device.  Mix_OpenAudio takes
           as its parameters the audio format we'd /like/ to have. */
  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
    std::cout << "Unable to open audio!\n" << std::endl;
  else
    sounding = true;

  if (sounding) {
    boing = Mix_LoadWAV("hit.wav");
    gong = Mix_LoadWAV("gong.wav");
    music = Mix_LoadMUS("ambient.wav");
    Mix_PlayMusic(music, -1);
  }

  sim = new Simulator(mSceneMgr);

  return ret;
}
//-------------------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
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
bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
  bool ret = BaseApplication::frameRenderingQueued(evt);

  if (paused) {
    slowdownval += 1/1800.f;
    Ogre::Entity* tile = tileEntities[0];
    tile->setMaterialName("Examples/Chrome");
  } else {
    Ogre::Entity* tile = tileEntities[0];
  }

  if(slowdownval <= 1/60.f) {
    bool hit = sim->simulateStep(slowdownval);
    if(hit && !gameDone) {
      if (sounding) {
        Mix_PlayChannel(-1, boing, 0);
        std::cout << "Playing impact noise." << std::endl;
      }

      tileEntities.back()->setMaterialName("Examples/BumpyMetal");

      if(tileEntities.size() > 0) {
        tileEntities.pop_back();
        tileSceneNodes.pop_back();
      }
      score++;

      if(tileEntities.size() == 0 && !gameDone) {
        gameDone = true;
        winTimer = 0;
        congratsPanel->show();

        for(int i = 0; i < balls.size(); i++) {
          balls[i]->enableGravity();
          globalBall->enableGravity();
          std::cout << "enabling gravity\n";
        }
      }
    }
  }

  if (gameDone) {
    winTimer++;

    if(winTimer > 360) {
      levelTearDown();
      levelSetup(currLevel);
      congratsPanel->hide();
    }
  }

  simonSaysAnim();

  if (isCharging) {
    if(chargeShot < 10000)
      chargeShot += 80;
    // std::cout << chargeShot << std::endl;
  } else
    chargeShot = 0;

  return ret;
}
//-------------------------------------------------------------------------------------
bool TutorialApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  isCharging = false;
  if(chargeShot >= 1000 && !gameDone) {
    if(globalBall != NULL) {
      sim->removeBall(globalBall);
      // delete globalBall->node;
    }

    int x = mCamera->getPosition().x;
    int y = mCamera->getPosition().y;
    int z = mCamera->getPosition().z;

    Ogre::Entity* ballMeshpc = mSceneMgr->createEntity("sphere.mesh");
    ballMeshpc->setCastShadows(true);

    Ogre::SceneNode* nodepc = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    nodepc->attachObject(ballMeshpc);
    globalBall = new Ball(nodepc, x, y, z, 100);
    sim->addBall(globalBall);
    double force = chargeShot;
    Ogre::Vector3 direction = mCamera->getOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z;
    globalBall->applyForce(force * direction.x, force * direction.y, force * direction.z);
    shotsFired++;
  }

  return BaseApplication::mouseReleased(arg, id);
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
  TutorialApplication app;

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
