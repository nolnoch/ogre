AACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= BaseGame.h TileGame.h Simulator.h TileSimulator.h BallManager.h CameraMan.h Ball.h SoundManager.h NetManager.h

bin_PROGRAMS= OgreApp
OgreApp_CPPFLAGS= -I$(top_srcdir)
OgreApp_SOURCES= BaseGame.cpp TileGame.cpp Simulator.cpp TileSimulator.cpp BallManager.cpp SoundManager.cpp NetManager.cpp
OgreApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS)
OgreApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) -lSDL -lSDL_mixer -lSDL_net

EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
