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


typedef int SoundFile;

class SoundManager {
public:
  SoundManager();
  virtual ~SoundManager();

  bool initSoundManager();
  bool loadMusic(const char *name);
  int loadSound(const char *name);
  int getVolume();
  void setVolume(double vol);
  void playMusic();
  void playSound(int chunk);
  void pauseMusic();
  void mute();
  void unmute();
  void toggleSound();

private:
  bool sounding, initialized, musicPlaying;
  Mix_Music *music;
  std::vector<Mix_Chunk *> chunks;
};

#endif /* SOUNDMANAGER_H_ */
