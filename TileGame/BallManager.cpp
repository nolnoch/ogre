/*
 * BallManager.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: nolnoch
 */

#include "BallManager.h"

BallManager::BallManager(TileSimulator *sim):
sim(sim),
globalBall(0),
globalBallActive(false)
{
}

BallManager::~BallManager() {
  clearBalls();
}

bool BallManager::initBallManager() {
  sim->setBallManager(this);

  return true;
}

void BallManager::setGlobalBall(Ball *ball) {
  globalBall = ball;
  globalBallActive = true;
}

Ball* BallManager::addBall(Ogre::SceneNode* n, int x, int y, int z, int r) {
  n->setPosition(x, y, z);

  btRigidBody *body = sim->addBallShape(n, r);
  Ball *ball = new Ball(body, n, x, y, z);
  ballList.push_back(ball);

  return ball;
}

Ball* BallManager::addMainBall(Ogre::SceneNode* n, int x, int y, int z, int r) {
  Ball *mainBall = addBall(n, x, y, z, r);
  mainBalls.push_back(mainBall);

  return mainBall;
}

void BallManager::enableGravity() {
  std::vector<Ball *>::iterator it;
  for (it = ballList.begin(); it != ballList.end(); it++) {
    (*it)->enableGravity();
  }
}

void BallManager::removeBall(Ball* rmBall) {
  bool found = false;
  std::vector<Ball *>::iterator it;
  for (it = ballList.begin(); it != ballList.end() && !found; it++) {
    if ((*it) == rmBall) {
      ballList.erase(it);
      found = true;
    }
  }

  btRigidBody* ballBody = rmBall->getRigidBody();
  sim->getDynamicsWorld().removeRigidBody(ballBody);
  delete ballBody->getMotionState();
  delete rmBall;
}

void BallManager::removeGlobalBall() {
  removeBall(globalBall);
  globalBall = NULL;
  globalBallActive = false;
}

bool BallManager::isGlobalBall() {
  return globalBallActive;
}

void BallManager::clearBalls() {
  std::vector<Ball *>::iterator it;
  for (it = ballList.begin(); it != ballList.end(); it++) {
    btRigidBody* ballBody = (*it)->getRigidBody();
    sim->getDynamicsWorld().removeRigidBody(ballBody);
    delete ballBody->getMotionState();
    delete (*it);
  }

  globalBall = NULL;
  globalBallActive = false;
  ballList.clear();
  mainBalls.clear();
}

TileSimulator* BallManager::getSimulator() {
  return sim;
}

bool BallManager::checkCollisions(btRigidBody *aTile, void *body0, void *body1) {
  bool hit = false;

  std::vector<Ball *>::iterator it;
  for (it = mainBalls.begin(); it != mainBalls.end() && !hit; it++) {
    Ball* mball = (*it);

    if ((aTile == body0 && mball->checkRigidBody((btRigidBody*)body1)) ||
        (aTile == body1 && mball->checkRigidBody((btRigidBody*)body0))) {
      mball->lockPosition();
      hit = true;
    }
  }

  return hit;
}
