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

class TutorialApplication : public BaseApplication
{
public:
	TutorialApplication(void);
	virtual ~TutorialApplication(void);
protected:
	virtual void createScene(void);
	// Ogre::FrameListener
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

private:
	//Update nodes' positions based on trackers
	bool processUnbufferedInput(const Ogre::FrameEvent& evt);
	//Handlers for tracker events
	static void VRPN_CALLBACK handleHMDTracker(void* userData, const vrpn_TRACKERCB t);

	//Data received from the trackers
	vrpn_TRACKERCB rightHandData, leftHandData, HMDData;

	//Tracker for left and right hands, and HMD
	vrpn_Tracker_Remote *tracker;

	//Nodes for each tracker
	Ogre::SceneNode* rightHandNode, *leftHandNode, *HMDNode;

};
#endif // #ifndef __TutorialApplication_h_

//---------------------------------------------------------------------------
