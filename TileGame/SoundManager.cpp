/*
 * SoundManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "SoundManager.h"

SoundManager::SoundManager():
sounding(true),
initialized(false),
musicPlaying(false),
music(0)
{
}

SoundManager::~SoundManager() {
  mute();

  Mix_FreeMusic(music);

  std::vector<Mix_Chunk *>::iterator it;
  for (it = chunks.begin(); it != chunks.end(); it++) {
    Mix_FreeChunk(*it);
  }
  chunks.clear();
}

bool SoundManager::initSoundManager() {
  // Initialize Audio [based on http://www.kekkai.org/roger/sdl/mixer/]
  /* We're going to be requesting certain things from our audio
             device, so we set them up beforehand */
  int audio_rate = 22050;
  Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
  int audio_channels = 2;
  int audio_buffers = 4096;

  /* This is where we open up our audio device.  Mix_OpenAudio takes
             as its parameters the audio format we'd /like/ to have. */
  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
    std::cout << "Unable to open audio!\n" << std::endl;
  else
    initialized = true;

  return initialized;
}

bool SoundManager::loadMusic(const char *name) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return false;
  }

  music = Mix_LoadMUS(name);

  return music;
}

int SoundManager::loadSound(const char *name) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return false;
  }

  int idx;
  Mix_Chunk *chunk;

  chunk = Mix_LoadWAV(name);
  if (!chunk) {
    std::cout << "SDL_mixer: Unable to load chunk sound file." << std::endl;
    idx = -1;
  } else {
    idx = chunks.size();
    chunks.push_back(chunk);
  }

  return idx;
}

void SoundManager::setVolume(int vol) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }

  Mix_Volume(-1, vol);
}

void SoundManager::playMusic() {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }

  if (sounding) {
    musicPlaying = true;
    Mix_PlayMusic(music, -1);
  }
}

void SoundManager::playSound(int chunkIdx) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }

  if (sounding)
    Mix_PlayChannel(-1, chunks[chunkIdx], 0);
}

void SoundManager::pauseMusic() {
  if (!initialized || !musicPlaying) {
    std::cout << "SoundManager: No music playing." << std::endl;
    return;
  }

  musicPlaying = false;

  Mix_HaltMusic();
}

void SoundManager::mute() {
  sounding = false;

  if (initialized)
    pauseMusic();
}

void SoundManager::unmute() {
  sounding = true;

  if (initialized)
    playMusic();
}

void SoundManager::toggleSound() {
  sounding ? mute() : unmute();
}



