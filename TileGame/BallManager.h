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
  std::vector<Ball *> playerBalls;

  BallManager(TileSimulator *sim);
  virtual ~BallManager();

  bool initBallManager();
  void initMultiplayer(int nPlayers);
  void setGlobalBall(Ball *ball);
  void setPlayerBall(Ball *ball, int idx);
  Ball* addBall(Ogre::SceneNode* n, int x, int y, int z, int r);
  Ball* addMainBall(Ogre::SceneNode* n, int x, int y, int z, int r);
  void enableGravity();
  void removeBall(Ball* rmBall);
  void removeGlobalBall();
  void removePlayerBall(int idx);
  bool isGlobalBall();
  bool isPlayerBall(int idx);
  void clearBalls();
  int getNumberBallCollisions();

  TileSimulator* getSimulator();

  bool checkCollisions(btRigidBody *aTile, void *body0, void *body1);

private:
  std::vector<Ball *> ballList;
  std::vector<Ball *> mainBalls;
  std::vector<bool> playerBallsActive;
  TileSimulator *sim;
  bool globalBallActive;
  int ballCollisions;
};

#endif /* BALLMANAGER_H_ */
