/*
 * Ball.h
 *
 *  Created on: Mar 3, 2014
 *      Author: nolnoch
 */

#ifndef BALL_H_
#define BALL_H_

#include <vector>

#include "OgreMotionState.h"


class BallManager;

class Ball {
  public:
    Ball(btRigidBody *rB, Ogre::SceneNode *n, int x, int y, int z):
      rigidBody(rB),
      node(n),
      mass(1)
  {
      rB->setAngularFactor(0.0f);
      rB->setRestitution(0.7);
      rB->setDamping(0.5, 0.0);
  }

    virtual ~Ball() {
      delete node;
      delete rigidBody;
    }

    void enableGravity() {
      rigidBody->setMassProps(1, btVector3(0, 0, 0));
      rigidBody->setGravity(btVector3(0, -980, 0));
      rigidBody->activate(true);
    }

    void lockPosition() {
      rigidBody->setMassProps(0, btVector3(0, 0, 0));
    }

    void unlockPosition() {
      rigidBody->setMassProps(mass, btVector3(0, 0, 0));
    }

    void setPosition(int x, int y, int z) {
      node->setPosition(x, y, z);
      rigidBody->getMotionState()->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1),
          btVector3(x, y, z)));
    }

    void applyForce(double dx, double dy, double dz) {
      rigidBody->activate(true);
      rigidBody->applyCentralImpulse(btVector3(dx, dy, dz));
    }

    bool checkRigidBody(btRigidBody* ptr) {
      return ptr == rigidBody;
    }

    btRigidBody* getRigidBody() {
      return rigidBody;
    }

    Ogre::SceneNode* getSceneNode() {
      return node;
    }

  private:
    Ogre::SceneNode* node;
    btScalar mass;
    btRigidBody* rigidBody;
  };

#endif /* BALL_H_ */
