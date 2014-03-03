
#include <OgreSceneManager.h>

class OgreMotionState : public btMotionState
{
    protected:
      Ogre::SceneNode* ogreObject;
      btTransform position;

    public:
      OgreMotionState(btTransform newposition, Ogre::SceneNode* object)
      {
          ogreObject = object;
          position = newposition;
      }

      void getWorldTransform(btTransform& worldTrans) const
      {
          worldTrans = position;
      }

      void setWorldTransform(const btTransform& worldTrans)
      {
          if(ogreObject == NULL)
              return;
          btQuaternion rot = worldTrans.getRotation();
          ogreObject->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
          btVector3 pos = worldTrans.getOrigin();
          ogreObject->setPosition(pos.x(), pos.y(), pos.z());
          position = worldTrans;
      }
};
