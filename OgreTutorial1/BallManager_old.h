/*
-----------------------------------------------------------------------------
Filename:    BallManager.h
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
 */
#ifndef __BallManager_h_
#define __BallManager_h_


#include "TileSimulator.h"
#include "Ball.h"


class BallManager_old {
public:
  Ball *globalBall;

  BallManager(TileSimulator *sim):
    sim(sim),
    globalBall(0)
  {
    ballList = new std::vector<Ball *>;
    sim->setBallManager(this);
  }

  virtual ~BallManager() {
    clearBalls();
    delete &ballList;
  }

  bool initBallManager() {
    bool ret = false;

    //TODO init.

    return ret;
  }

  void setGlobalBall(Ball *ball) {
    globalBall = ball;
  }

  Ball* addBall(Ogre::SceneNode* n, int x, int y, int z, int r) {
    btRigidBody *body = sim->addBallShape(n, r);
    Ball *ball = new Ball(body, n, x, y, z);
    ballList.push_back(ball);

    return ball;
  }

  Ball* addMainBall(Ogre::SceneNode* n, int x, int y, int z, int r) {
    Ball *mainBall = addBall(n, x, y, z, r);
    mainBalls.push_back(mainBall);

    return mainBall;
  }

  void enableGravity() {
    std::vector<Ball *>::iterator it;
    for (it = ballList.begin(); it != ballList.end(); it++) {
      (*it)->enableGravity();
    }
  }

  void removeBall(Ball* rmBall) {
    bool found = false;
    std::vector<Ball *>::iterator it;
    for (it = ballList.begin(); it != ballList.end() && !found; it++) {
      if ((*it) == rmBall) {
        ballList.erase(it);
        found = true;
      }
    }

    btRigidBody* ballBody = rmBall->getRigidBody();
    sim->getDynamicsWorld()->removeRigidBody(ballBody);
    delete ballBody->getMotionState();
    delete ballBody;
    delete rmBall->getSceneNode();
  }

  void clearBalls() {
    std::vector<Ball *>::iterator it;
    for (it = ballList.begin(); it != ballList.end(); it++) {
      delete *it;
    }

    ballList.clear();
  }

  TileSimulator& getSimulator() {
    return sim;
  }

  bool checkCollisions(btRigidBody *aTile, void *body0, void *body1) {
    bool hit = false;

    std::vector<Ball *>::iterator it;
    for (it = ballList.begin(); it != ballList.end() && !hit; it++) {
      Ball* mball = (*it);

      if ((aTile == body0 && mball->checkRigidBody((btRigidBody*)body1)) ||
          (aTile == body1 && mball->checkRigidBody((btRigidBody*)body0))) {
        mball->lockPosition();
        hit = true;
      }
    }

    return hit;
  }

private:
  std::vector<Ball *> ballList;
  std::vector<Ball *> mainBalls;
  TileSimulator *sim;
};

#endif // #ifndef __BallManager_h_
