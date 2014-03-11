/*
 * BallManager.h
 *
 *  Created on: Mar 4, 2014
 *      Author: nolnoch
 */

#ifndef BALLMANAGER_H_
#define BALLMANAGER_H_

#include <vector>

#include "TileSimulator.h"
#include "Ball.h"

class TileSimulator;

class BallManager {
public:
  Ball *globalBall;

  BallManager(TileSimulator *sim);
  virtual ~BallManager();

  bool initBallManager();
  void setGlobalBall(Ball *ball);
  Ball* addBall(Ogre::SceneNode* n, int x, int y, int z, int r);
  Ball* addMainBall(Ogre::SceneNode* n, int x, int y, int z, int r);
  void enableGravity();
  void removeBall(Ball* rmBall);
  void removeGlobalBall();
  bool isGlobalBall();
  void clearBalls();

  TileSimulator* getSimulator();

  bool checkCollisions(btRigidBody *aTile, void *body0, void *body1);

private:
  std::vector<Ball *> ballList;
  std::vector<Ball *> mainBalls;
  TileSimulator *sim;
  bool globalBallActive;
};

#endif /* BALLMANAGER_H_ */
