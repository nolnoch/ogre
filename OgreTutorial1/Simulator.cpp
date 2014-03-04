#include "Simulator.h"


Simulator::Simulator():
collisionConfiguration(0),
dispatcher(0),
broadphase(0),
solver(0),
dynamicsWorld(0),
sceneMgr(0)
{
}

void Simulator::initSimulator() {
  collisionConfiguration = new btDefaultCollisionConfiguration();
  dispatcher = new btCollisionDispatcher(collisionConfiguration);
  broadphase = new btDbvtBroadphase();
  solver = new btSequentialImpulseConstraintSolver();

  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
  dynamicsWorld->setGravity(btVector3(0, 0, 0));
}

void Simulator::createBounds(const int offset) {
  addPlaneBound(0, 1, 0, -offset);
  addPlaneBound(0, -1, 0, -offset);
  addPlaneBound(1, 0, 0, -offset);
  addPlaneBound(-1, 0, 0, -offset);
  addPlaneBound(0, 0, 1, -offset);
  addPlaneBound(0, 0, -1, -offset);
}

bool Simulator::simulateStep(double delay) {
  bool ret = false;

  dynamicsWorld->stepSimulation((1/60.f) - delay, 10);

  return ret;
}

void Simulator::addPlaneBound(int x, int y, int z, int d) {
  btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(x, y, z), d);
  btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
  btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
  btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);

  groundRigidBody->setRestitution(1.0);
  groundRigidBody->setCollisionFlags(groundRigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
  dynamicsWorld->addRigidBody(groundRigidBody);
}

btRigidBody& Simulator::addBoxShape(Ogre::SceneNode* node, int xsize, int ysize, int zsize)  {
  btCollisionShape* boxShape = new btBoxShape(btVector3(xsize, ysize, zsize));
  btDefaultMotionState* boxMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1),
      btVector3(node->_getDerivedPosition().x, node->_getDerivedPosition().y, node->_getDerivedPosition().z)));
  btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, boxMotionState, boxShape, btVector3(0, 0, 0));
  btRigidBody* boxRigidBody = new btRigidBody(boxRigidBodyCI);

  boxRigidBody->setRestitution(1.0);
  boxRigidBody->setCollisionFlags(boxRigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
  dynamicsWorld->addRigidBody(boxRigidBody);

  collisionShapes.push_back(boxShape);

  return boxRigidBody;
}

void Simulator::registerCallback(void &func) {
  gContactProcessedCallback = func;
}
