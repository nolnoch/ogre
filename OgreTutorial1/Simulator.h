#include <bullet/btBulletDynamicsCommon.h>
#include <vector>

#include "BallManager.h"


using std::vector;


class Simulator {

public:
  Simulator(Ogre::SceneManager* sM);
  ~Simulator();

  void createBounds(const int offset);
  bool simulateStep(double delay);

private:
  btDefaultCollisionConfiguration* collisionConfiguration;
  btBroadphaseInterface* broadphase;
  btCollisionDispatcher* dispatcher;
  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;

  vector<btCollisionShape *> collisionShapes;

  Ogre::SceneManager* sceneMgr;
  vector<Ball*> balls;
  std::deque<btRigidBody*> tiles;

  BallManager *ballManager;

  static btRigidBody* activetile;
  static vector<Ball*> mainballs;
  static bool targethit;

  void addPlaneBound(int x, int y, int z, int d);
  void addTile(Ogre::SceneNode* node, int xsize, int ysize, int zsize);
};
