#include "Simulator.h"


TileSimulator::TileSimulator(Ogre::SceneManager *sM) {
  sceneMgr = sM;

  Simulator::initSimulator();
  ballManager = new BallManager(sM);
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

btRigidBody& TileSimulator::addBoxShape(Ogre::SceneNode *n, int x, int y, int z)  {
  btRigidBody *box = Simulator::addBoxShape(n, x, y, z);

  registerCallback((void &) tileCallback);

  activetile = box;
  tiles.push_back(box);

  return box;
}
