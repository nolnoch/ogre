ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= BaseApplication.h TutorialApplication.h Simulator.h TileSimulator.h BallManager.h CameraMan.h Ball.h

bin_PROGRAMS= OgreApp
OgreApp_CPPFLAGS= -I$(top_srcdir)
OgreApp_SOURCES= BaseApplication.cpp TutorialApplication.cpp Simulator.cpp TileSimulator.cpp BallManager.cpp
OgreApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS) -I/usr/include/bullet
OgreApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) -lBulletDynamics -lBulletCollision -lLinearMath -lSDL -lSDL_mixer

EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
