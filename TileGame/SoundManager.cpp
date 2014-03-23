/*
 * SoundManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "SoundManager.h"

std::vector<Sound> SoundManager::activeSounds(0);
void channelDoneWrapper(int);
// Registers the callback
void channelDoneWrapper(int channel) {
  std::cout << " callback! " << std::endl;
  SoundManager manager;
  manager.channelDone(channel);
}

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
    // Register a callback for when a sound stops playing.


  std::vector<Mix_Chunk *>::iterator it;
  for (it = chunks.begin(); it != chunks.end(); it++) {
    Mix_FreeChunk(*it);
  }
  chunks.clear();
}

// Don't call any sdl methods while in a callback
void SoundManager::channelDone(int channel) {
  for(int i = 0; i < activeSounds.size(); i++) {
    Sound *s = &(activeSounds[i]);
    if(s->channel == channel) {
      std::cout << " channel " << channel << " array size " << activeSounds.size() <<  std::endl;
      s->active = false;
    }
  }
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

int SoundManager::getVolume() {
  return Mix_Volume(-1, -1);
}

void SoundManager::setVolume(double vol) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }
  int volume = MIX_MAX_VOLUME;
  if (((int) vol) < volume)
    volume = (int) (vol * MIX_MAX_VOLUME);

  Mix_Volume(-1, volume);
  Mix_Volume(0, volume);
  Mix_Volume(1, volume);
  Mix_VolumeMusic(volume);
}

void SoundManager::lowerVolume()
{
    double volume = getVolume();
    volume -= 8;
    if(volume < 0)
      volume = 0;
    setVolume(volume / MIX_MAX_VOLUME);
}

void SoundManager::raiseVolume()
{
    double volume = getVolume();
    volume += 8;
    if(volume > MIX_MAX_VOLUME)
      volume = MIX_MAX_VOLUME;
    setVolume(volume / MIX_MAX_VOLUME);
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


void SoundManager::playSound(int chunkIdx, Ogre::Vector3 soundPosition, Ogre::Camera* mCamera) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }

  if (sounding) {
    Mix_ChannelFinished(channelDoneWrapper);
    int channel = Mix_PlayChannel(-1, chunks[chunkIdx], 0);
    int dist = calcDistance(mCamera->getPosition(), soundPosition);
    // put this sound in our list of active sounds.
    Sound s;
    s.soundPosition = soundPosition;
    s.chunk = chunks[chunkIdx];
    s.distance = dist;
    s.channel = channel;
    s.active = true;
    activeSounds.push_back(s);

    // Initialize sound position
    int rightIntensity = calcPanning(mCamera, soundPosition);
    Mix_SetPanning(s.channel, 254 - rightIntensity, rightIntensity);
    Mix_SetDistance(s.channel, dist);
  }
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


void SoundManager::updateSounds(Ogre::Camera* mCamera) {

  // Remove any sounds from our list of active sounds that might not be active.
  for(int i = 0; i < activeSounds.size(); i++) {
    Sound *s = &(activeSounds[i]);
    if(s->active == false) {
      // Reset sound position on the given channel, and remove the sound
      Mix_SetPanning(s->channel, 255, 255);
      Mix_SetDistance(s->channel, 0);
      activeSounds.erase(activeSounds.begin() + i);
    }
  }

  // update sound position for all active sounds
  for(int i = 0; i < activeSounds.size(); i++) {
    Sound *s = &(activeSounds[i]);
    int rightIntensity = calcPanning(mCamera, s->soundPosition);
    Mix_SetPanning(s->channel, 254 - rightIntensity, rightIntensity);
    int dist = calcDistance(mCamera->getPosition(), s->soundPosition);
    s->distance = dist;
    Mix_SetDistance(s->channel, dist);
  }

}

/*
 * Calculates sound level by getting the distance of two vectors and mapping it between 0 and 255
 * using a given range (5000)
 */
int SoundManager::calcDistance(Ogre::Vector3 camPosition, Ogre::Vector3 soundPosition) {
   Ogre::Real distance = camPosition.distance(soundPosition); //get the distance between sound and camera
    if(distance > 5500)
      distance = 5500;
    int dist = distance/5500 * 255; //1500 should be the max range.
    return dist;
}

// Returns volume for the right ear.
// If the sound is directly to the right, that channel gets 254 of intensity.
// If the sound is directly in front of us, each channel gets 254/2 of intensity.
int SoundManager::calcPanning(Ogre::Camera* mCamera, Ogre::Vector3 soundPosition) {
    Ogre::Vector3 camDirection = mCamera->getOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z;
    Ogre::Vector3 soundDir = soundPosition - mCamera->getPosition();
    soundDir.normalise();
    // Retrieve the angle made between the sound position and the camera's direction,
    // which should be pointing forwards.
    Ogre::Quaternion q = camDirection.getRotationTo(soundDir);
    Ogre::Radian radians = q.getYaw();

    // anything left of the camera returns positive degrees
    float degrees = radians.valueDegrees();

    int leftIntensity;
    int rightIntensity;

    // from 0 to -179 it's the right ear.
    if(degrees <= 0) {
      //std::cout << "degrees " << degrees << std::endl;
      degrees *= -1;
      if(degrees > 90)
          degrees = 90 - (degrees - 90); // if 91, we subtract 1 and go back to 89
      // Intensity can never be 255 because then the effect cancels out.
      int intensity = (degrees / 90.0) * 254/2 + 254/2;
      //std::cout << "intensity " << intensity << std::endl;
      rightIntensity = intensity;
      leftIntensity = 255 - (rightIntensity);
    }
    // from 0 to 179 it's the left ear.
    else {
      if(degrees > 90)
          degrees = 90 - (degrees - 90); 
      int intensity = (degrees / 90) * 254/2 + 254/2;
      //std::cout << "intensity " << intensity << std::endl;
      leftIntensity = intensity;
      rightIntensity = 254 - (leftIntensity);
    }
    return rightIntensity;
}




/*
0 = directly in front.
90 = directly to the right.
180 = directly behind.
270 = directly to the left.
*/
