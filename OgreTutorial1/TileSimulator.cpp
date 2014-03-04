#include "TileSimulator.h"


TileSimulator::TileSimulator() {
  this->initSimulator();
}

TileSimulator::~TileSimulator() {

}

void TileSimulator::initSimulator() {
  Simulator::initSimulator();
}

bool TileSimulator::simulateStep(double delay) {
  targethit = Simulator::simulateStep(delay);

  if(targethit) {
    if(tiles.size() > 0) {
      tiles.pop_back();

      if(tiles.size() > 0)
        activetile = tiles.back();
      else
        activetile = NULL;
    }
  }

  return targethit;
}

btRigidBody* TileSimulator::addTile(Ogre::SceneNode *n, int x, int y, int z)  {
  btRigidBody *box = Simulator::addBoxShape(n, x, y, z);

  tiles.push_back(box);
  activetile = box;

  registerCallback((void *) tileCallback);

  return box;
}

btRigidBody* TileSimulator::addBallShape(Ogre::SceneNode *n, int r)  {
  btRigidBody *ball = Simulator::addBallShape(n, r, 1);

  return ball;
}

void TileSimulator::setBallManager(BallManager *bM) {
  tileBallMgr = bM;
}

bool TileSimulator::tileCallback(btManifoldPoint& cp, void *body0, void *body1) {
  if (!activetile)
    return true;

  return tileBallMgr->checkCollisions(activetile, body0, body1);
}
