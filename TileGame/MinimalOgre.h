/*
-----------------------------------------------------------------------------
Filename:    MinimalOgre.h
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
#ifndef __MinimalOgre_h_
#define __MinimalOgre_h_

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>
#include "Simulator.h"
#include "CameraMan.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>


class MinimalOgre : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
    MinimalOgre(void);
    virtual ~MinimalOgre(void);
    bool go(void);
    Ogre::RenderWindow * getWindow(void) { return mWindow; }
    Ogre::Timer * getTimer(void) { return mTimer; }
    OIS::Mouse * getMouse(void) { return mMouse; }
    OIS::Keyboard * getKeyboard(void) { return mKeyboard; }
protected:
    const static int WALL_SIZE = 2400;
    const static int PLANE_DIST = WALL_SIZE / 2; //the initial offset from the center
    const static int NUM_TILES_ROW = 5; // number of tiles in each row of a wall.
    const static int NUM_TILES_WALL = NUM_TILES_ROW * NUM_TILES_ROW; //number of total tiles on a wall.
    const static int TILE_WIDTH = WALL_SIZE / NUM_TILES_ROW;
    bool gameStart;
    int currLevel;
    int currTile;
    bool animDone;

    bool gameDone;
    int winTimer;
    int tileCounter;
    int chargeShot;
    bool isCharging;

    Ogre::Timer timer;
    Ogre::Light* panelLight;

    std::deque<Ogre::Entity *> allTileEntities;
    std::deque<Ogre::SceneNode *> tileList;
    std::deque<Ogre::Entity *> tileEntities;
    std::deque<Ogre::SceneNode *> tileSceneNodes;

    vector<Ball*> balls;
    Ball* globalBall;

    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
    Ogre::Timer *mTimer;
    Ogre::AnimationState *mState;

    //Ogre::Vector3 mDirection;
    //Ogre::Real mSpeed;
    //Ogre::Sphere ballBound;
    Ogre::PlaneBoundedVolume boxBound;
    Simulator* sim;
    int score;
    int shotsFired;

    Ogre::Plane wallUp;
    Ogre::Plane wallDown;
    Ogre::Plane wallBack;
    Ogre::Plane wallFront;
    Ogre::Plane wallLeft;
    Ogre::Plane wallRight;

    Ogre::Vector3 vZero;

    bool paused;
    double slowdownval;

    bool sounding;
    Mix_Chunk *boing, *gong;
    Mix_Music *music;

    // OgreBites
    OgreBites::SdkTrayManager* mTrayMgr;
    CameraMan* mCameraMan;      // basic camera controller
    OgreBites::ParamsPanel* mDetailsPanel;    // sample details panel
    OgreBites::ParamsPanel* scorePanel;
    OgreBites::Label* congratsPanel;
    OgreBites::Label* chargePanel;
    bool mCursorWasVisible;                   // was cursor visible before dialog appeared
    bool mShutDown;

    // OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;

    // Level Setup
    void levelSetup(int num);
    void levelTearDown();
    void ballSetup(int cubeSize);

    void simonSaysAnim();


    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);



    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    // Ogre::WindowEventListener
    virtual void windowResized(Ogre::RenderWindow* rw);
    virtual void windowClosed(Ogre::RenderWindow* rw);
};

#endif // #ifndef __MinimalOgre_h_
