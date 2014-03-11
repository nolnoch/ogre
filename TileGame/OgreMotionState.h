/*
-----------------------------------------------------------------------------
Filename:    OgreMotionState.h
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
 */
#ifndef __OgreMotionState_h_
#define __OgreMotionState_h_


#include <bullet/btBulletDynamicsCommon.h>
#include <OgreSceneManager.h>


class OgreMotionState : public btMotionState {
protected:
  Ogre::SceneNode* ogreObject;
  btTransform position;

public:
  OgreMotionState(btTransform newposition, Ogre::SceneNode* object) {
    ogreObject = object;
    position = newposition;
  }

  void getWorldTransform(btTransform& worldTrans) const {
    worldTrans = position;
  }

  void setWorldTransform(const btTransform& worldTrans) {
    if(!ogreObject)
      return;

    btQuaternion rot = worldTrans.getRotation();
    ogreObject->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
    btVector3 pos = worldTrans.getOrigin();
    ogreObject->setPosition(pos.x(), pos.y(), pos.z());
    position = worldTrans;
  }
};

#endif // #ifndef __OgreMotionState_h_
