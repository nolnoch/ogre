#include <bullet/btBulletDynamicsCommon.h>
#include <vector>

#include "BallManager.h"


class TileSimulator : public Simulator {

public:
  static btRigidBody *activetile;
  static std::vector<Ball *> mainballs;
  static bool targethit;

  TileSimulator(Ogre::SceneManager *sM);
  virtual ~TileSimulator();

  virtual bool simulateStep(double delay);
  virtual btRigidBody& addBoxShape(Ogre::SceneNode *n, int x, int y, int z);

  static bool tileCallback(btManifoldPoint& cp, void *body0, void *body1) {
    bool ret = false;

    if (!activetile)
      return true;

    for (int i = 0; i < mainballs.size(); i++) {
      Ball* mball = mainballs[i];

      if (activetile == body0 && mball->checkRigidBody((btRigidBody*)body1)) {
        targethit = true;
        mball->lockPosition();
        return true;
      } else if (activetile == body1 && mball->checkRigidBody((btRigidBody*)body0)) {
        targethit = true;
        mball->lockPosition();
        return true;
      }
    }

    return ret;
  }

private:
  std::vector<Ball *> balls;
  std::deque<btRigidBody *> tiles;

  BallManager *ballManager;
};
