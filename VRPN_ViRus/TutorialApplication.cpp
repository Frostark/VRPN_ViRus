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

	//Load the map

	//Set fog
	Ogre::ColourValue fadeColour(.9, .9, .9);
	mWindow->getViewport(0)->setBackgroundColour(fadeColour);

	mSceneMgr->setFog(Ogre::FOG_EXP2, fadeColour, 0.1);

	// Create an Entity
	Ogre::Entity* ogreMap = mSceneMgr->createEntity("Map", "virus_map.mesh");

	// Create a SceneNode and attach the Entity to it
	Ogre::SceneNode* mapNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("MapNode");
	mapNode->setPosition(Ogre::Vector3(-17, 0.0, 16));
	mapNode->attachObject(ogreMap);

	// Add collision detection to it
	OgreBulletCollisions::CollisionShape *floorShape;
	floorShape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3::UNIT_Y, 0); // (normal vector, distance)

	// A rigid body is needed for the shape
	OgreBulletDynamics::RigidBody *floorBody = new OgreBulletDynamics::RigidBody("FloorBody", mWorld);
	floorBody->setStaticShape(floorShape, 0.1, 0.8); // (shape, restitution, friction)

	// Push the created objects to the deques
	ViRus::Hittable *floorHittable = new ViRus::HitObstacle(floorBody, floorShape, mapNode);
	hitmap.add_hittable(*floorBody->getBulletObject(), *floorHittable);

	debugDrawer = new OgreBulletCollisions::DebugDrawer();
	debugDrawer->setDrawWireframe(true); // we want to see the Bullet shapes

	mWorld->setDebugDrawer(debugDrawer);
	mWorld->setShowDebugShapes(true); // enable it if you want to see the Bullet shapes
	Ogre::SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("debugDrawer", Ogre::Vector3::ZERO);
	node->attachObject(static_cast <Ogre::SimpleRenderable *> (debugDrawer));

	//Create the camera that will be controlled by the HMD
	/*Ogre::Camera* newCamera = mSceneMgr->createCamera("NewCam");
	mWindow->getViewport(0)->setCamera(newCamera);
	newCamera->setPosition(Ogre::Vector3(0, 0, 0));
	newCamera->lookAt(Ogre::Vector3(Ogre::Vector3::NEGATIVE_UNIT_Z));
	newCamera->setNearClipDistance(0.1);*/

	mCamera->setPosition(Ogre::Vector3::ZERO + player_pos);
	mCameraMan->setTopSpeed(3);

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

	HMDNode->setPosition(player_pos);

	//Attach an eye node to the HMD node, and translate the offset from the tracer's position to the eye position
	Ogre::SceneNode *eyeNode = HMDNode->createChildSceneNode();
	eyeNode->translate(0.0, -0.1, 0.05);
	//eyeNode->attachObject(newCamera);//Attach the camera to the eyeNode

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

	//Guns

	ViRus::Gun::hitmap = &hitmap;
	ViRus::Gun::mWorld = mWorld;
	ViRus::Gun::ptr_scn_mgr = mSceneMgr;

	//Left gun
	left_gun = new ViRus::Gun(leftHandNode, "Barrel.mesh");

	//Right gun
	right_gun = new ViRus::Gun(rightHandNode, "Barrel.mesh");

	//Spawner

	ViRus::Spawner::hitmap = &hitmap;
	ViRus::Spawner::mWorld = mWorld;
	ViRus::Spawner::ptr_scn_mgr = mSceneMgr;

	constexpr int MAX_ENEMIES = 5;
	constexpr int ENE_HEALTH = 50;
	constexpr int ENE_DMG = 10;
	constexpr float ENE_TIME_ATTACK = 2.0;
	constexpr float ENE_VEL = 4;
	constexpr float ENE_SCALE = 1.8 / 200.0;
	constexpr float ENE_RESTITUTION = 0.1;
	constexpr float ENE_FRICTION = 5.0;
	constexpr float ENE_MASS = 10.0;
	spawner = new ViRus::Spawner(Ogre::Vector3::ZERO, 10, MAX_ENEMIES, ViRus::TeamType::ENEMY, ENE_HEALTH, ENE_DMG, ENE_TIME_ATTACK, ENE_VEL, "ninja.mesh", ENE_SCALE, ENE_RESTITUTION, ENE_FRICTION, ENE_MASS);
	spawner->set_callback(spawner_callback);


	static constexpr double PENGUIN_SCALING = 0.04;

	// Define the penguin mesh
	Ogre::Entity* penguin = mSceneMgr->createEntity("Penguin", "penguin.mesh");
	penguin->setCastShadows(true);
	Ogre::SceneNode *penguinNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("PenguinNode");
	Ogre::SceneNode *penguinEntityNode = penguinNode->createChildSceneNode("PenguinEntity");
	penguinEntityNode->attachObject(penguin);
	penguinEntityNode->yaw(Ogre::Degree(180));

	// We need the bounding box of the entity to be able to set the size of the Bullet shape
	Ogre::AxisAlignedBox penguinBoundingBox = penguin->getBoundingBox();

	// Size of the Bullet shape, a box
	Ogre::Vector3 penguinShapeSize = Ogre::Vector3::ZERO;
	penguinShapeSize = penguinBoundingBox.getSize();
	penguinShapeSize /= 2.0f; // Only the half needed
	penguinShapeSize *= 0.96f; // Bullet margin is a bit bigger so we need a smaller size

	penguinNode->scale(PENGUIN_SCALING, PENGUIN_SCALING, PENGUIN_SCALING); // The penguin is too big for us
	penguinShapeSize *= PENGUIN_SCALING; // don't forget to scale down the Bullet shape too

	penguinNode->yaw(Ogre::Degree(180));

	penguinNode->translate(0.0, 24 * PENGUIN_SCALING, 0.0);

	// After that create the Bullet shape with the calculated size
	OgreBulletCollisions::BoxCollisionShape *penguinShape;
	penguinShape = new OgreBulletCollisions::BoxCollisionShape(penguinShapeSize);


	// and the Bullet rigid body
	OgreBulletDynamics::RigidBody *penguinBody = new OgreBulletDynamics::RigidBody("penguinBody", mWorld);
	Ogre::Vector3 penguinPosition = penguinNode->getPosition();
	Ogre::Quaternion penguinOrientation = penguinNode->getOrientation();
	penguinBody->setShape(penguinNode, penguinShape, 0.1f, 5.0f, 10.0f, // (node, shape, restitution, friction, mass
		penguinPosition, penguinOrientation); // starting position, orientation)

	penguinBody->getBulletRigidBody()->setAngularFactor(btVector3(0, 0, 0));
	penguinNode->translate(Ogre::Vector3(0, 0, -5));

	// Push the created objects to the deques

	ptr_target = new ViRus::HitCharAttack(penguinBody, penguinShape, penguinNode, ViRus::TeamType::ENEMY, 40, 20);
	ptr_target->set_callback(target_callback);

	hitmap.add_hittable(*penguinBody->getBulletObject(), *ptr_target);

	//Character physics

	//Add the collider to the head
	Ogre::Vector3 HMD_cylinder_size(0.25, 2, 0.25);
	HMD_cylinder_size *= 0.5;
	
	OgreBulletCollisions::CylinderCollisionShape *HMDCylinder = new OgreBulletCollisions::CylinderCollisionShape(HMD_cylinder_size, Ogre::Vector3::UNIT_Y);

	OgreBulletDynamics::RigidBody *HMDbody = new OgreBulletDynamics::RigidBody("HMDbody", mWorld);
	Ogre::Vector3 HMDpos = HMDNode->getPosition() + Ogre::Vector3(0, -HMD_cylinder_size.y, 0);
	Ogre::Quaternion HMDrot = HMDNode->getOrientation();
	HMDbody->setStaticShape(HMDNode, HMDCylinder, 0.6, 0.6,HMDpos,HMDrot);
	HMDbody->setKinematicObject(true);
	HMDbody->disableDeactivation();

	ptr_hero = new ViRus::HitPlayer(HMDbody, HMDCylinder, HMDNode, ViRus::TeamType::HERO, 100);
	ptr_hero->set_callback(target_callback);
	ptr_hero->set_at_death(at_death_callback);

	hitmap.add_hittable(*HMDbody->getBulletObject(), *ptr_hero);

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
	//vrpnButton2->mainloop();

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

	while (spawner->need_spawn())
		spawner->spawn();

	hitmap.clean_queued();

	if (ptr_hero&&ptr_target)
		ptr_target->chase(*ptr_hero);
	if (ptr_hero&&spawner)
		spawner->chase(*ptr_hero);

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
	right_gun->refresh(evt.timeSinceLastFrame);

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
void TutorialApplication::target_callback(ViRus::Hittable *h)
{
	if (h == ptr_target)
		ptr_target = nullptr;
	else if (h == ptr_hero)
		ptr_hero = nullptr;
}
bool TutorialApplication::at_death_callback(ViRus::HitPlayer *player)
{
	player->revive();

	return false;
}
void TutorialApplication::spawner_callback(ViRus::Hittable * h)
{
	spawner->callback(h);
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
