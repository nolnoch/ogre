#include <bullet/btBulletDynamicsCommon.h>
#include <vector>

#include "BallManager.h"


extern ContactProcessedCallback gContactProcessedCallback;

using std::vector;

class Simulator {

public:
  Simulator();
  virtual ~Simulator();

  virtual void initSimulator();
  virtual bool simulateStep(double delay);
  virtual btRigidBody& addBoxShape(Ogre::SceneNode* n, int x, int y, int z);
  virtual void registerCallback(void &func);

private:
  btDefaultCollisionConfiguration* collisionConfiguration;
  btBroadphaseInterface* broadphase;
  btCollisionDispatcher* dispatcher;
  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;

  vector<btCollisionShape *> collisionShapes;

  Ogre::SceneManager* sceneMgr;

  virtual void addPlaneBound(int x, int y, int z, int d);
  virtual void createBounds(const int offset);
};
