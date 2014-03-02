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
    sounding(false)
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

  if (sounding)
    boing = Mix_LoadWAV("blip.wav");

  return ret;
}
//-------------------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
  // Create the visible mesh ball with initial velocity.
  Ogre::Entity* ballMesh = mSceneMgr->createEntity("Ball", "sphere.mesh");
  ballMesh->setMaterialName("Examples/SphereMappedRustySteel");
  ballMesh->setCastShadows(true);
  mDirection = Ogre::Vector3(-0.3, 0.6, -0.9);
  mSpeed = 300.0f;

  // Create the bounding geometry, used only in collision testing.
  ballBound = Ogre::Sphere(vZero, 200);
  boxBound = Ogre::PlaneBoundedVolume(Ogre::Plane::NEGATIVE_SIDE);
  boxBound.planes.push_back(wallBack = Ogre::Plane(Ogre::Vector3::UNIT_Z, -800));
  boxBound.planes.push_back(wallFront = Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Z, -800));
  boxBound.planes.push_back(wallDown = Ogre::Plane(Ogre::Vector3::UNIT_Y, -800));
  boxBound.planes.push_back(wallUp = Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Y, -800));
  boxBound.planes.push_back(wallLeft = Ogre::Plane(Ogre::Vector3::UNIT_X, -800));
  boxBound.planes.push_back(wallRight = Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_X, -800));

  // Use the planes from above to generate new meshes for walls.
  Ogre::MeshManager::getSingleton().createPlane("ground",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallDown,
      1600, 1600, 20, 20, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);
  Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entGround);
  entGround->setMaterialName("Custom/texture_blend");
  entGround->setCastShadows(false);

  Ogre::MeshManager::getSingleton().createPlane("ceiling",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallUp,
      1600, 1600, 20, 20, true, 1, 2, 2, Ogre::Vector3::UNIT_Z);
  Ogre::Entity* entCeiling = mSceneMgr->createEntity("CeilingEntity", "ceiling");
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entCeiling);
  entCeiling->setMaterialName("Examples/CloudySky");
  entCeiling->setCastShadows(false);

  Ogre::MeshManager::getSingleton().createPlane("back",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallBack,
      1600, 1600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entBack = mSceneMgr->createEntity("BackEntity", "back");
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entBack);
  entBack->setMaterialName("Examples/Rockwall");
  entBack->setCastShadows(false);

  Ogre::MeshManager::getSingleton().createPlane("front",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallFront,
      1600, 1600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entFront = mSceneMgr->createEntity("FrontEntity", "front");
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entFront);
  entFront->setMaterialName("Examples/Rockwall");
  entFront->setCastShadows(false);

  Ogre::MeshManager::getSingleton().createPlane("left",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallLeft,
      1600, 1600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entLeft = mSceneMgr->createEntity("LeftEntity", "left");
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entLeft);
  entLeft->setMaterialName("Examples/Rockwall");
  entLeft->setCastShadows(false);

  Ogre::MeshManager::getSingleton().createPlane("right",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallRight,
      1600, 1600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entRight = mSceneMgr->createEntity("RightEntity", "right");
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entRight);
  entRight->setMaterialName("Examples/Rockwall");
  entRight->setCastShadows(false);

  // Attach the node.
  headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
  headNode->attachObject(ballMesh);

  // Set ambient light
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.05, 0.05, 0.05));

  // Create a light
  Ogre::Light* lSun = mSceneMgr->createLight("SunLight");
  lSun->setType(Ogre::Light::LT_POINT);
  lSun->setDiffuseColour(0.95, 0.95, 1.00);
  lSun->setPosition(0,1400,0);
  lSun->setAttenuation(2250, 1.0, 0.0000000001, 0.000001);
}
//-------------------------------------------------------------------------------------
bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
  bool ret = BaseApplication::frameRenderingQueued(evt);

  /********************************************************************
   * Animation
   */

  Ogre::Vector3 point = headNode->getPosition();
  Ogre::Real adjust = 0.0;
  bool found = false;

  // Given a bounding box, we can easily test each plane in the PlaneList.
  for (int i = 0; i < 6 && !found; i++) {
    Ogre::Real dist = boxBound.planes[i].getDistance(ballBound.getCenter());
    if (dist < 100.01) {
      mDirection = mDirection.reflect(boxBound.planes[i].normal);
      adjust = 100.5 - dist;
      found = true;
      if (sounding)
        Mix_PlayChannel(-1, boing, 0);
    }
  }

  // Add distance traveled plus collision adjustment, and update position.
  point = point + (((evt.timeSinceLastFrame * mSpeed) + adjust) * mDirection);
  ballBound.setCenter(point);
  headNode->setPosition(point);

  /*******************************************************************/

  return ret;
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
