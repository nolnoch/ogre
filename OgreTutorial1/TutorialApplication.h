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
  virtual void createScene(void);
  Ogre::Timer *mTimer;
  Ogre::SceneNode* headNode;

  Ogre::Vector3 mDirection;
  Ogre::Real mSpeed;

  Ogre::Sphere ballBound;
  Ogre::PlaneBoundedVolume boxBound;

  Ogre::Plane wallUp;
  Ogre::Plane wallDown;
  Ogre::Plane wallBack;
  Ogre::Plane wallFront;
  Ogre::Plane wallLeft;
  Ogre::Plane wallRight;

  Ogre::Vector3 vZero;
};

#endif // #ifndef __TutorialApplication_h_
