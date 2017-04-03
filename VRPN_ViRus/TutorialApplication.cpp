/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
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

#include "TutorialApplication.hpp"

//---------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//---------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
}

//---------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
	//Head's position
	static const Ogre::Vector3 player_pos(0, 2, 0);

	//Set up the scene manager on the map
	ViRus::Hittable::ptr_scn_mgr = mSceneMgr;

	mNumEntitiesInstanced = 0; // how many shapes are created

	mWorld = new OgreBulletDynamics::DynamicsWorld(mSceneMgr,
		Ogre::AxisAlignedBox(Ogre::Vector3(-10000, -10000, -10000), Ogre::Vector3(10000, 10000, 10000)), //aligned box for Bullet
		Ogre::Vector3(0, -9.81, 0)); // gravity vector for Bullet

	//Create the camera that will be controlled by the HMD
	Ogre::Camera* newCamera = mSceneMgr->createCamera("NewCam");
	mWindow->getViewport(0)->setCamera(newCamera);
	newCamera->setPosition(Ogre::Vector3(0, 0, 0));
	newCamera->lookAt(Ogre::Vector3(Ogre::Vector3::NEGATIVE_UNIT_Z));
	newCamera->setNearClipDistance(0.1);

	//tracker = new vrpn_Tracker_Remote("iotracker@161.67.196.59:3883");
	//tracker->register_change_handler(this, handleHMDTracker);

	vrpnButton1 = new vrpn_Button_Remote("joyWin32_1@localhost");
	//vrpnButton2 = new vrpn_Button_Remote("joyWin32_2@localhost");
	vrpnButton1->register_change_handler(this, handleButton1);
	//vrpnButton2->register_change_handler(this, handleButton2);

	//Init tracking data
	vrpn_TRACKERCB blank = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

	rightHandData = blank;
	leftHandData = blank;
	HMDData = blank;

	// Set the scene's ambient light
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));

	// Create the hand entities
	//Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
	Ogre::Entity* leftHandEntity = mSceneMgr->createEntity("weaponLeftHand.mesh");
	Ogre::Entity* rightHandEntity = mSceneMgr->createEntity("weaponRightHand.mesh");

	// Create the nodes

	//Create the HMD node
	HMDNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("HMDNode");

	HMDNode->setPosition(Ogre::Vector3::ZERO + player_pos);

	//Attach an eye node to the HMD node, and translate the offset from the tracer's position to the eye position
	Ogre::SceneNode *eyeNode = HMDNode->createChildSceneNode();
	eyeNode->translate(0.0, -0.1, 0.05);
	eyeNode->attachObject(newCamera);//Attach the camera to the eyeNode

	//Create the left hand node, and attach the entity to it
	leftHandNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("leftHandNode");
	leftHandNode->attachObject(leftHandEntity);
	leftHandNode->scale(Ogre::Vector3(1, 1, 1));
	//leftHandNode->pitch(Ogre::Degree(-90.0f));
	//leftHandNode->roll(Ogre::Degree(180.0f));

	leftHandNode->setPosition(Ogre::Vector3(-0.2, -0.2, -0.4)+player_pos);

	//Create the right hand node, and attach
	rightHandNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("rightHandNode");
	rightHandNode->attachObject(rightHandEntity);
	rightHandNode->scale(Ogre::Vector3(1, 1, 1));
	//rightHandNode->pitch(Ogre::Degree(-90.0f));
	//rightHandNode->roll(Ogre::Degree(180.0f));

	rightHandNode->setPosition(Ogre::Vector3(+0.2, -0.2, -0.4)+player_pos);


	// Create a Light and set its position
	Ogre::Light* light = mSceneMgr->createLight("MainLight");
	light->setPosition(20.0f, 80.0f, 50.0f);


	//Floor
	// Define a floor plane mesh
	Ogre::Plane floorPlane;
	floorPlane.normal = Ogre::Vector3::UNIT_Y;
	floorPlane.d = 0.0;
	Ogre::MeshManager::getSingleton().createPlane("FloorPlane",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		floorPlane, 1000, 1000, 20, 20, true, 1, 50, 50, Ogre::Vector3::UNIT_Z);

	// Create the entity (the floor)
	Ogre::Entity* floor = mSceneMgr->createEntity("Floor", "FloorPlane");
	floor->setMaterialName("Examples/BumpyMetal");
	floor->setCastShadows(false);
	Ogre::SceneNode * floorNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	floorNode->attachObject(floor);

	// Add collision detection to it
	OgreBulletCollisions::CollisionShape *floorShape;
	floorShape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3::UNIT_Y, 0); // (normal vector, distance)

																								// A rigid body is needed for the shape
	OgreBulletDynamics::RigidBody *floorBody = new OgreBulletDynamics::RigidBody("FloorBody", mWorld);
	floorBody->setStaticShape(floorShape, 0.1, 0.8); // (shape, restitution, friction)

													 // Push the created objects to the deques

	ViRus::Hittable *floorHittable = new ViRus::HitObstacle(floorBody, floorShape, floorNode);
	hitmap.add_hittable(*floorBody->getBulletObject(), *floorHittable);

	//Guns

	//Left gun
	left_gun = new ViRus::Gun(leftHandNode, "Barrel.mesh");

	//Right gun
	right_gun = new ViRus::Gun(rightHandNode, "Barrel.mesh");
}

void TutorialApplication::destroyScene(void)
{
	BaseApplication::destroyScene();

	hitmap.clear_all();

	delete mWorld->getDebugDrawer();
	mWorld->setDebugDrawer(nullptr);
	delete mWorld;

	delete left_gun;
	delete right_gun;
}

//-------------------------------------------------------------------------------------
//Update while swapping buffers
bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	bool ret = BaseApplication::frameRenderingQueued(evt);

	//Tracker loops
	//tracker->mainloop();
	vrpnButton1->mainloop();

	if (!processUnbufferedInput(evt)) return false;

	return ret;
}

//Update nodes' positions based on trackers
bool TutorialApplication::processUnbufferedInput(const Ogre::FrameEvent& evt)
{
	//HMDNode->setPosition(Ogre::Vector3(HMDData.pos[0], HMDData.pos[1], HMDData.pos[2]));
	//HMDNode->setOrientation(HMDData.quat[3], HMDData.quat[0], HMDData.quat[1], HMDData.quat[2]);

	//rightHandNode->setPosition(Ogre::Vector3(rightHandData.pos[0], rightHandData.pos[1], rightHandData.pos[2]));
	//rightHandNode->setOrientation(rightHandData.quat[3], rightHandData.quat[0], rightHandData.quat[1], rightHandData.quat[2]);

	//leftHandNode->setPosition(Ogre::Vector3(leftHandData.pos[0], leftHandData.pos[1], leftHandData.pos[2]));
	//leftHandNode->setOrientation(leftHandData.quat[3], leftHandData.quat[0], leftHandData.quat[1], leftHandData.quat[2]);


	// Update Bullet Physics animation
	mWorld->stepSimulation(evt.timeSinceLastFrame);

	btDynamicsWorld * mBulletWorld = mWorld->getBulletDynamicsWorld();
	int numManifolds = mBulletWorld->getDispatcher()->getNumManifolds();
	for (int i = 0;i<numManifolds;i++)
	{
		btPersistentManifold* contactManifold = mBulletWorld->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());

		hitmap.handle_collision(obA, obB);
	}

	hitmap.clean_queued();

	if (shotLeft)
	{
		left_gun->fire();
		shotLeft = false;
	}

	if (shotRight)
	{
		right_gun->fire();
		shotRight= false;
	}

	left_gun->refresh(evt.timeSinceLastFrame);

	return true;
}

void VRPN_CALLBACK TutorialApplication::handleButton1(void* userData, const vrpn_BUTTONCB b)
{
	if (b.button == 0 && b.state)
		static_cast<TutorialApplication *>(userData)->shotLeft = true;
}

void VRPN_CALLBACK TutorialApplication::handleButton2(void* userData, const vrpn_BUTTONCB b)
{
	if (b.button == 0 & b.state)
		static_cast<TutorialApplication *>(userData)->shotRight = true;
}

void VRPN_CALLBACK TutorialApplication::handleHMDTracker(void* userData, const vrpn_TRACKERCB t)
{
	vrpn_TRACKERCB *pData = nullptr;
	switch (t.sensor)
	{
	
		case 3:
			pData = &(((TutorialApplication*)userData)->HMDData);
			break;

		case 1:
			pData = &(((TutorialApplication*)userData)->rightHandData);
			break;

		case 4:
			pData = &(((TutorialApplication*)userData)->leftHandData);
			break;
	}

	if (pData)
	{
		*pData = t;

		//From mm to m
		pData->pos[0] /= 1000;
		pData->pos[1] /= 1000;
		pData->pos[2] /= 1000;
	}
}
//-------------------------------------------------------------------------------------

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        TutorialApplication app;

        try {
            app.go();
        } catch(Ogre::Exception& e)  {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occurred: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
