/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.h
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
Tutorial Framework (for Ogre 1.9)
http://www.ogre3d.org/wiki/
-----------------------------------------------------------------------------
*/

#ifndef __TutorialApplication_h_
#define __TutorialApplication_h_

#include "BaseApplication.hpp"

//---------------------------------------------------------------------------

#include "vrpn_Tracker.h"
#include "vrpn_Button.h"

#include "Physics.hpp"
#include "Gameplay.hpp"

#include "OgreBulletDynamicsRigidBody.h" // for OgreBullet
#include "Shapes/OgreBulletCollisionsStaticPlaneShape.h" // for static planes
#include "Shapes/OgreBulletCollisionsBoxShape.h" // for boxes
#include "Shapes/OgreBulletCollisionsCylinderShape.h" // for cylinders

ViRus::HitPlayer *ptr_hero;
ViRus::Spawner *spawner;
ViRus::Gun *left_gun;
ViRus::Gun *right_gun;

class TutorialApplication : public BaseApplication
{
public:
	TutorialApplication(void);
	virtual ~TutorialApplication(void);

	bool shotRight = false, shotLeft = false;

protected:
	virtual void createScene(void);
	virtual void destroyScene(void);
	// Ogre::FrameListener
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

private:

	//Update nodes' positions based on trackers
	bool processUnbufferedInput(const Ogre::FrameEvent& evt);
	static void VRPN_CALLBACK handleButton1(void * userData, const vrpn_BUTTONCB b);
	static void VRPN_CALLBACK handleButton2(void * userData, const vrpn_BUTTONCB b);
	//Handlers for tracker events
	static void VRPN_CALLBACK handle_iotracker_tracker(void* userData, const vrpn_TRACKERCB t);
	static void VRPN_CALLBACK handle_HMD_tracker(void* userData, const vrpn_TRACKERCB t);
	static void VRPN_CALLBACK handle_leftHand_tracker(void* userData, const vrpn_TRACKERCB t);
	static void VRPN_CALLBACK handle_rightHand_tracker(void* userData, const vrpn_TRACKERCB t);

	//Data received from the trackers
	vrpn_TRACKERCB rightHandData, leftHandData, HMDData;

	//Tracker for left and right hands, and HMD
	vrpn_Tracker_Remote *tracker=nullptr;
	vrpn_Tracker_Remote *HMD_tracker=nullptr,*leftHand_tracker=nullptr,*rightHand_tracker=nullptr;

	vrpn_Button_Remote *vrpnButton1, *vrpnButton2;

	//Nodes for each tracker
	Ogre::SceneNode* rightHandNode, *leftHandNode, *HMDNode;

	// Bullet Physics engine
	OgreBulletDynamics::DynamicsWorld *mWorld; // OgreBullet World
	OgreBulletCollisions::DebugDrawer *debugDrawer;
	int mNumEntitiesInstanced;

	ViRus::HitMap hitmap;

	static void target_callback(ViRus::Hittable *);
	static bool at_death_callback(ViRus::HitPlayer *player);
	static void spawner_callback(ViRus::Hittable * h);
	static void left_gun_callback(ViRus::Hittable *h);
	static void right_gun_callback(ViRus::Hittable *h);
	static void pickup_callback(ViRus::Hittable *h);
};
#endif // #ifndef __TutorialApplication_h_

//---------------------------------------------------------------------------
