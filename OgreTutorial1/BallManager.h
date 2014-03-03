#include "OgreMotionState.h"
#include <vector>

#ifndef BULLET_LIBRARY
#include <bullet/btBulletDynamicsCommon.h>
#define BULLET_LIBRARY
#endif


class Ball;

class BallManager {
public:
  BallManager(Ogre::SceneManager *sM, btDynamicsWorld *dW):
    sceneMgr(sM),
    dynWorld(dW)
{
    ballList = new std::vector<Ball *>;

    motionState = new OgreMotionState(btTransform(btQuaternion(0, 0, 0, 1.0), btVector3(x, y, z)), node);
    collisionShape = new btSphereShape(radius);
}

  ~BallManager() {
    for (auto *ball : ballList) {
      delete ball;
    }
    delete ballList;
  }

  bool initBallManager() {
    bool ret = false;

    //TODO init.

    return ret;
  }

  Ball* addBall() {
    Ball *ball = new Ball();

    dynWorld->addRigidBody(rigidBody);
    ballList.push_back(ball);

    return ball;
  }

  void addMainBall() {
    mainBalls.push_back(addBall());
  }

  void Simulator::removeBall(Ball* ball) {
    btRigidBody* body = ball->getRigidBody();
    dynamicsWorld->removeRigidBody(body);
    delete body->getMotionState();
    delete body;
    delete ball->node;
  }

private:
  std::vector<Ball *> ballList;
  std::vector<Ball *> mainBalls;

  Ogre::SceneManager *sceneMgr;
  btDynamicsWorld *dynWorld;


  class Ball {
  public:
    Ball(Ogre::SceneNode* newnode, int x, int y, int z, int r):
      radius(r),
      node(newnode),
      mass(1),
      collisionShape(0),
      rigidBody(0),
      motionState(0)
    {
      btVector3 sphereInertia(0, 0, 0);
      collisionShape->calculateLocalInertia(mass, sphereInertia);
      btRigidBody::btRigidBodyConstructionInfo ballCI(mass, motionState, collisionShape, sphereInertia);
      rigidBody = new btRigidBody(ballCI);
      rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
      rigidBody->setAngularFactor(0.0f);
      rigidBody->setRestitution(0.7);
      rigidBody->setDamping(0.5, 0.0);

      node->setPosition(x, y, z);
    }

    void enableGravity() {
      rigidBody->setMassProps(1, btVector3(0, 0, 0));
      rigidBody->setGravity(btVector3(0, -980, 0));
      rigidBody->activate(true);
    }

    void lockPosition() {
      rigidBody->setMassProps(0, btVector3(0, 0, 0));
    }

    void setPosition(int x, int y, int z) {
      node->setPosition(x, y, z);
      rigidBody->setMassProps(0, btVector3(0, 0, 0));
      motionState->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));
    }

    void applyForce(double dx, double dy, double dz) {
      rigidBody->activate(true);
      rigidBody->applyCentralImpulse(btVector3(dx, dy, dz));
    }

    btRigidBody* getRigidBody() {
      return rigidBody;
    }

  private:
    Ogre::SceneNode* node;
    int radius;
    btScalar mass;
    btCollisionShape* collisionShape;
    btRigidBody* rigidBody;
    OgreMotionState* motionState;
  };

};
