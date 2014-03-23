/*
 * SoundManager.h
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_


#include <vector>
#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
 #include "BaseGame.h"



typedef int SoundFile;

typedef struct {
   Ogre::Vector3 soundPosition;
   Mix_Chunk * chunk;
   int distance; // this is the volume, which is based on the distance it is from a vector
   int channel; // channel this sound is being played at.
   bool active;
} Sound;

class SoundManager {
public:
  SoundManager();
  virtual ~SoundManager();
  bool initSoundManager();
  bool loadMusic(const char *name);
  int loadSound(const char *name);
  int getVolume();
  void setVolume(double vol);
  void raiseVolume();
  void lowerVolume();
  void playMusic();
  void playSound(int chunk);
  void playSound(int chunk, Ogre::Vector3 soundPosition, Ogre::Camera* mCamera);
  void pauseMusic();
  void mute();
  void unmute();
  void toggleSound();
  void updateSounds(Ogre::Camera* mCamera);
//  void updatesounds(Ogre::Vector3 camPosition);
  void channelDone(int channel);
private:
  bool sounding, initialized, musicPlaying;
  Mix_Music *music;
  std::vector<Mix_Chunk *> chunks;
  static std::vector<Sound> activeSounds;
  // Calculates sound attenuation using the distance of two vectors
  int calcDistance(Ogre::Vector3 camPosition, Ogre::Vector3 soundPosition);
  // Calculates sound orientation
  int calcPanning(Ogre::Camera* mCamera, Ogre::Vector3 soundPosition);
};




#endif /* SOUNDMANAGER_H_ */
