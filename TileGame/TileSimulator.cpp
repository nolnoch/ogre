#include "TileSimulator.h"


TileSimulator::TileSimulator() {
}

TileSimulator::~TileSimulator() {

}

void TileSimulator::initSimulator() {
  Simulator::initSimulator();
}

bool TileSimulator::simulateStep(double delay) {
  Simulator::simulateStep(delay);
  bool ret = targethit;

  if (targethit) {
    if (tiles.size() > 1) {
      tiles.pop_back();
      activetile = tiles.back();
    } else if (!tiles.empty()) {
      tiles.pop_back();
    } else
      activetile = NULL;

    targethit = false;
  }

  return ret;
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
  if (!activetile || targethit)
    return true;

  targethit = tileBallMgr->checkCollisions(activetile, body0, body1);

  return targethit;
}
