/*
-----------------------------------------------------------------------------
Filename:    TileSimulator.h
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
 */
#ifndef __TileSimulator_h_
#define __TileSimulator_h_

#include <bullet/btBulletDynamicsCommon.h>
#include <OgreSceneManager.h>
#include <vector>

#include "Simulator.h"
#include "BallManager.h"


class BallManager;
class Ball;

static btRigidBody *activetile;
static BallManager *tileBallMgr;
static bool targethit;

class TileSimulator : public Simulator {

public:
  TileSimulator();
  virtual ~TileSimulator();

  virtual void initSimulator();
  virtual bool simulateStep(double delay);
  virtual btRigidBody* addBallShape(Ogre::SceneNode *n, int r);
  btRigidBody* addTile(Ogre::SceneNode *n, int x, int y, int z);
  void setBallManager(BallManager *bM);
  void clearTiles();

  static bool tileCallback(btManifoldPoint& cp, void *body0, void *body1);

private:
  std::deque<btRigidBody *> tiles;
};

#endif // #ifndef __TileSimulator_h_
