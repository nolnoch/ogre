/*
-----------------------------------------------------------------------------
Filename:    Simulator.h
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
 */
#ifndef __Simulator_h_
#define __Simulator_h_

#include <bullet/btBulletDynamicsCommon.h>
#include <OgreSceneManager.h>
#include <vector>

#include "OgreMotionState.h"


extern ContactProcessedCallback gContactProcessedCallback;

class Simulator {

public:
  Ogre::SceneManager* sceneMgr;

  Simulator();
  virtual ~Simulator();

  virtual void initSimulator();
  virtual void createBounds(const int offset);
  virtual void registerCallback(void * func);
  virtual bool simulateStep(double delay);
  virtual void addPlaneBound(int x, int y, int z, int d);
  virtual btRigidBody* addBoxShape(Ogre::SceneNode* n, int x, int y, int z);
  virtual btRigidBody* addBallShape(Ogre::SceneNode* n, int r, int m);

  virtual btDiscreteDynamicsWorld& getDynamicsWorld();

private:
  btDefaultCollisionConfiguration* collisionConfiguration;
  btBroadphaseInterface* broadphase;
  btCollisionDispatcher* dispatcher;
  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;

  std::vector<btCollisionShape *> collisionShapes;
};

#endif // #ifndef __Simulator_h_
