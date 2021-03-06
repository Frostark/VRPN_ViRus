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
	app = this;

	hud = new OgreText();
								   // Now it is possible to use the Ogre::String as parameter too
	hud->setPos(0.2, 0.1f);        // Text position, using relative co-ordinates
	hud->setCol(1.0f, 0, 0, 1);    // Text colour (Red, Green, Blue, Alpha)

	points = 0;

	constexpr bool USING_IOTRACKER = false;

	ogre2dManager_radar = new Ogre2dManager();
	ogre2dManager_dot = new Ogre2dManager();
	ogre2dManager_damage = new Ogre2dManager();

	ogre2dManager_radar->init(mSceneMgr, Ogre::RENDER_QUEUE_OVERLAY, true);
	ogre2dManager_dot->init(mSceneMgr, Ogre::RENDER_QUEUE_OVERLAY, true);
	ogre2dManager_damage->init(mSceneMgr, Ogre::RENDER_QUEUE_OVERLAY, true);

	//Sounds
	sound_mgr = SoundManager::createManager();
	sound_mgr->init();
	sound_mgr->setAudioPath("..\\..\\media\\sounds\\");

	sound_mgr->loadAudio("fire_gun.wav", &audio_fire_gun, false);
	sound_mgr->loadAudio("enemy_hit.wav", &audio_enemy_hit, false);
	sound_mgr->loadAudio("enemy_dead.wav", &audio_enemy_dead, false);
	sound_mgr->loadAudio("player_hit.wav", &audio_player_hit, false);
	sound_mgr->loadAudio("player_dead.wav", &audio_player_dead, false);
	sound_mgr->loadAudio("music_cemetery.wav", &music_cemetery, true);
	sound_mgr->loadAudio("healing.wav", &audio_healing, false);

	if (audioEnabled)
		sound_mgr->playAudio(music_cemetery, true);

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

	mSceneMgr->setFog(Ogre::FOG_EXP2, fadeColour, 0.1 );

	// Create an Entity
	Ogre::Entity* ogreMap = mSceneMgr->createEntity("Map", "virus_map.mesh");

	// Create a SceneNode and attach the Entity to it
	Ogre::SceneNode* mapNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("MapNode");

	constexpr float MAPSCALE = 1;

	mapNode->scale(Ogre::Vector3(MAPSCALE, 1, MAPSCALE));
	mapNode->translate(Ogre::Vector3(-17*MAPSCALE, 0.0, 16*MAPSCALE));
	mapNode->attachObject(ogreMap);

	/*******************************************************************/
	/*****************************CREATE MENU***************************/
	/*******************************************************************/

	ViRus::Menu::ptr_scn_mgr = mSceneMgr;
	ViRus::Menu::hitmap = &hitmap;
	ViRus::Menu::mWorld = mWorld;
	
	menu = new ViRus::Menu("menu.mesh");

	drawMenu();

	/*******************************************************************/


	// Add collision detection to it
	OgreBulletCollisions::CollisionShape *floorShape;
	floorShape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3::UNIT_Y, 0); // (normal vector, distance)

	// A rigid body is needed for the shape
	OgreBulletDynamics::RigidBody *floorBody = new OgreBulletDynamics::RigidBody("FloorBody", mWorld, ViRus::ColliderType::OBSTACLE, ViRus::ColliderType::ENEMY | ViRus::ColliderType::HERO|ViRus::ColliderType::POWERUP);
	floorBody->setStaticShape(floorShape, 0.1, 0.8); // (shape, restitution, friction)

	// Push the created objects to the deques
	ViRus::Hittable *floorHittable = new ViRus::HitObstacle(floorBody, floorShape, mapNode);
	hitmap.add_hittable(*floorBody->getBulletObject(), *floorHittable);

	//Create the camera that will be controlled by the HMD
	Ogre::Camera* newCamera = mSceneMgr->createCamera("NewCam");
	mWindow->getViewport(0)->setCamera(newCamera);
	newCamera->setPosition(Ogre::Vector3(0, 0, 0));
	newCamera->lookAt(Ogre::Vector3(0, 0, -1));
	newCamera->setNearClipDistance(0.1);

	if (USING_IOTRACKER)
	{
		tracker = new vrpn_Tracker_Remote("iotracker@161.67.196.59:3883");
		tracker->register_change_handler(this, handle_iotracker_tracker);
	}
	else
	{
		HMD_tracker = new vrpn_Tracker_Remote("T6002@161.67.196.44:3883");
		HMD_tracker->register_change_handler(this, handle_HMD_tracker);

		leftHand_tracker = new vrpn_Tracker_Remote("Left_hand@161.67.196.44:3883");
		leftHand_tracker->register_change_handler(this, handle_leftHand_tracker);

		rightHand_tracker = new vrpn_Tracker_Remote("Right_hand@161.67.196.44:3883");
		rightHand_tracker->register_change_handler(this, handle_rightHand_tracker);
	}

	vrpnButton1 = new vrpn_Button_Remote("joyWin32_1@localhost");
	vrpnButton2 = new vrpn_Button_Remote("joyWin32_2@localhost");
	vrpnButton1->register_change_handler(this, handleButton1);
	vrpnButton2->register_change_handler(this, handleButton2);

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

	//Attach an eye node to the HMD node, and translate the offset from the tracer's position to the eye position
	Ogre::SceneNode *eyeNode = HMDNode->createChildSceneNode();
	eyeNode->translate(0.0, -0.1, 0.05);
	eyeNode->attachObject(newCamera);//Attach the camera to the eyeNode

	//Create the left hand node, and attach the entity to it
	leftHandNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("leftHandNode");
	leftHandNode->attachObject(leftHandEntity);
	leftHandNode->scale(Ogre::Vector3(1, 1, 1));
	leftHandNode->pitch(Ogre::Degree(-90.0f));
	leftHandNode->roll(Ogre::Degree(180.0f));

	//Create the right hand node, and attach
	rightHandNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("rightHandNode");
	rightHandNode->attachObject(rightHandEntity);
	rightHandNode->scale(Ogre::Vector3(1, 1, 1));
	rightHandNode->pitch(Ogre::Degree(-90.0f));
	rightHandNode->roll(Ogre::Degree(180.0f));


	// Create a Light and set its position
	Ogre::Light* light = mSceneMgr->createLight("MainLight");
	light->setPosition(20.0f, 80.0f, 50.0f);

	//Guns

	ViRus::Gun::hitmap = &hitmap;
	ViRus::Gun::mWorld = mWorld;
	ViRus::Gun::ptr_scn_mgr = mSceneMgr;

	//Left gun
	left_gun = new ViRus::Gun(leftHandNode, leftHandEntity->getBoundingBox().getSize().z*0.75, "bullet.mesh",1);
	left_gun->set_callback(left_gun_callback);

	//Right gun
	right_gun = new ViRus::Gun(rightHandNode, rightHandEntity->getBoundingBox().getSize().z*0.75, "bullet.mesh",1);
	right_gun->set_callback(right_gun_callback);

	//Spawner

	ViRus::Spawner::hitmap = &hitmap;
	ViRus::Spawner::mWorld = mWorld;
	ViRus::Spawner::ptr_scn_mgr = mSceneMgr;

	constexpr int MAX_ENEMIES = 5;
	constexpr int ENE_HEALTH = 50;
	constexpr int ENE_DMG = 10;
	constexpr float ENE_TIME_ATTACK = 1.0;
	constexpr float ENE_VEL = 1.5;
	constexpr float ENE_SCALE = 1.8 / 200.0;
	constexpr float ENE_RESTITUTION = 0.1;
	constexpr float ENE_FRICTION = 5.0;
	constexpr float ENE_MASS = 10.0;
	constexpr float ENE_MAX_WAIT_TIME = 4;
	spawner = new ViRus::Spawner(Ogre::Vector3::ZERO, 10.0*MAPSCALE, MAX_ENEMIES, ViRus::TeamType::ENEMY, ENE_HEALTH, ENE_DMG, ENE_TIME_ATTACK, ENE_VEL, "ninja.mesh", ENE_SCALE, ENE_RESTITUTION, ENE_FRICTION, ENE_MASS, ENE_MAX_WAIT_TIME, 0.4, "medkit.mesh");
	spawner->set_callback(spawner_callback);
	spawner->set_pickup_callback(pickup_callback);

	//Character physics

	//Add the collider to the head
	Ogre::Vector3 HMD_cylinder_size(1.5, 2, 1.5);
	HMD_cylinder_size /= 2.0;
	OgreBulletCollisions::CylinderCollisionShape *HMDCylinder = new OgreBulletCollisions::CylinderCollisionShape(HMD_cylinder_size, Ogre::Vector3::UNIT_Y);

	OgreBulletDynamics::RigidBody *HMDbody = new OgreBulletDynamics::RigidBody("HMDbody", mWorld, ViRus::ColliderType::HERO, ViRus::ColliderType::ENEMY | ViRus::ColliderType::POWERUP | ViRus::ColliderType::OBSTACLE);
	Ogre::Vector3 HMDpos = HMDNode->getPosition() + Ogre::Vector3(0, -HMD_cylinder_size.y, 0);
	Ogre::Quaternion HMDrot = HMDNode->getOrientation();
	HMDbody->setStaticShape(HMDNode, HMDCylinder, 0.6, 0.6,HMDpos,HMDrot);
	HMDbody->setKinematicObject(true);
	HMDbody->disableDeactivation();

	ptr_hero = new ViRus::HitPlayer(HMDbody, HMDCylinder, HMDNode, ViRus::TeamType::HERO, 100,damages,radar);
	ptr_hero->set_callback(target_callback);
	ptr_hero->set_at_death(at_death_callback);

	hitmap.add_hittable(*HMDbody->getBulletObject(), *ptr_hero);

	Ogre::TextureManager::getSingleton().load("damageIndicator.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::TextureManager::getSingleton().load("radar.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::TextureManager::getSingleton().load("enemyDot.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

void TutorialApplication::destroyScene(void)
{
	BaseApplication::destroyScene();

	hitmap.clear_all();

	delete mWorld->getDebugDrawer();
	mWorld->setDebugDrawer(nullptr);
	delete mWorld;

	ogre2dManager_damage->end();
	delete ogre2dManager_damage;
	ogre2dManager_dot->end();
	delete ogre2dManager_dot;
	ogre2dManager_radar->end();
	delete ogre2dManager_radar;

	delete left_gun;
	delete right_gun;
}

//-------------------------------------------------------------------------------------
//Update while swapping buffers
bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	bool ret = BaseApplication::frameRenderingQueued(evt);

	//Tracker loops

	if (tracker)
		tracker->mainloop();
	else
	{
		HMD_tracker->mainloop();
		leftHand_tracker->mainloop();
		rightHand_tracker->mainloop();
	}
	vrpnButton1->mainloop();
	vrpnButton2->mainloop();

	if (!processUnbufferedInput(evt)) return false;

	return ret && running;
}

//Update nodes' positions based on trackers
bool TutorialApplication::processUnbufferedInput(const Ogre::FrameEvent& evt)
{

	HMDNode->setPosition(Ogre::Vector3(HMDData.pos[0], HMDData.pos[1], HMDData.pos[2]));
	HMDNode->setOrientation(HMDData.quat[3], HMDData.quat[0], HMDData.quat[1], HMDData.quat[2]);

	rightHandNode->setPosition(Ogre::Vector3(rightHandData.pos[0], rightHandData.pos[1], rightHandData.pos[2]));
	rightHandNode->setOrientation(rightHandData.quat[3], rightHandData.quat[0], rightHandData.quat[1], rightHandData.quat[2]);

	leftHandNode->setPosition(Ogre::Vector3(leftHandData.pos[0], leftHandData.pos[1], leftHandData.pos[2]));
	leftHandNode->setOrientation(leftHandData.quat[3], leftHandData.quat[0], leftHandData.quat[1], leftHandData.quat[2]);


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

	if (spawner&&inGame)
		while (spawner->need_spawn())
			spawner->spawn();

	hitmap.clean_queued();

	if (spawner)
	{
		if (ptr_hero)
			spawner->chase(*ptr_hero);

		spawner->deltaTime(evt.timeSinceLastFrame);
	}

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

	
	if (ptr_hero&&inGame)
	{
		damages.update_ttl(evt.timeSinceLastFrame);
		damages.draw(*ogre2dManager_damage);
		radar.draw_radar(*ogre2dManager_radar);

		ptr_hero->update_di();
		ptr_hero->update_rdr();

		if (spawner)
			spawner->draw_radar(radar, *ogre2dManager_dot);

		hud->setText(Ogre::String("Health ") + Ogre::StringConverter().toString(ptr_hero ? ptr_hero->get_health() : 0) + Ogre::String("         Points ") + Ogre::StringConverter().toString(points));
	}

	else
		hud->setText("");

	if (points >= MAX_POINTS)
		TutorialApplication::at_death_callback(ptr_hero);

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

void VRPN_CALLBACK TutorialApplication::handle_iotracker_tracker(void* userData, const vrpn_TRACKERCB t)
{
	vrpn_TRACKERCB *pData = nullptr;
	switch (t.sensor)
	{
	
		case 2:
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
void VRPN_CALLBACK TutorialApplication::handle_HMD_tracker(void * userData, const vrpn_TRACKERCB t)
{
	(((TutorialApplication*)userData)->HMDData) = t;
}
void VRPN_CALLBACK TutorialApplication::handle_leftHand_tracker(void * userData, const vrpn_TRACKERCB t)
{
	(((TutorialApplication*)userData)->leftHandData) = t;
}
void VRPN_CALLBACK TutorialApplication::handle_rightHand_tracker(void * userData, const vrpn_TRACKERCB t)
{
	(((TutorialApplication*)userData)->rightHandData) = t;
}
void TutorialApplication::drawMenu()
{
	menu->show();
	menu->addButton("goButton.mesh", Ogre::Vector3(8, 2.3, -4), at_go_callback);
	menu->addButton("soundButton.mesh", Ogre::Vector3(8, 2.3, -1), at_sound_callback);
	menu->addButton("quitButton.mesh", Ogre::Vector3(8, 2.3, 2), at_quit_callback);
}
void TutorialApplication::target_callback(ViRus::Hittable *h)
{
	if (h == ptr_hero)
		ptr_hero = nullptr;
}
bool TutorialApplication::at_death_callback(ViRus::HitPlayer *player)
{
	inGame = false;
	player->revive();


	// Create an QuitButton Entity
	Ogre::Entity* ogreButton = app->mSceneMgr->createEntity("DeadPanel" , points>=MAX_POINTS?"congratulations.mesh":"dead.mesh");
	Ogre::SceneNode* buttonNode = app->mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Ogre::SceneNode* entNode = buttonNode->createChildSceneNode();
	entNode->attachObject(ogreButton);

	buttonNode->scale(Ogre::Vector3(0.1, 0.1, 0.1));

	Ogre::Vector3 size = ogreButton->getBoundingBox().getSize()*0.5*0.1;

	entNode->translate(Ogre::Vector3(-size.x, -size.y, 0), Ogre::Node::TS_WORLD);

	Ogre::Vector3 pos_panel(6, 2.5, 0);

	buttonNode->rotate(Vector3::NEGATIVE_UNIT_Y, Degree(90));
	buttonNode->translate(pos_panel);

	Ogre::Vector3 position = buttonNode->getPosition();
	Ogre::Quaternion rot = buttonNode->getOrientation();

	OgreBulletCollisions::BoxCollisionShape *box = new OgreBulletCollisions::BoxCollisionShape(size);
	OgreBulletDynamics::RigidBody *body = new OgreBulletDynamics::RigidBody("Body" , app->mWorld, ViRus::ColliderType::OBSTACLE, ViRus::ColliderType::HERO);
	body->setStaticShape(buttonNode, box, 1, 1, position, rot);
	body->setKinematicObject(true);
	body->disableDeactivation();

	ViRus::HitButton *ptr_button = new ViRus::HitButton(body, box, buttonNode);

	ptr_button->set_at_button(at_deadPanel_callback);

	app->hitmap.add_hittable(*body->getBulletObject(), *ptr_button);

	//drawMenu();
	points = 0;
	if (spawner)
		spawner->kill_all();

	return false;
}
void TutorialApplication::spawner_callback(ViRus::Hittable * h)
{
	spawner->callback(h);
}
void TutorialApplication::left_gun_callback(ViRus::Hittable * h)
{
	if (left_gun)
		left_gun->callback(h);
}
void TutorialApplication::right_gun_callback(ViRus::Hittable * h)
{
	if (right_gun)
		right_gun->callback(h);
}
void TutorialApplication::pickup_callback(ViRus::Hittable * h)
{
	spawner->medkit_callback(h);
}
void TutorialApplication::at_go_callback(ViRus::HitButton * h)
{
	inGame = true;
	menu->hide();
}
void TutorialApplication::at_quit_callback(ViRus::HitButton * h)
{
	running = false;
}

void TutorialApplication::at_sound_callback(ViRus::HitButton * h)
{
	if (audioEnabled = !audioEnabled)
		sound_mgr->playAudio(music_cemetery, true);
	else
		sound_mgr->stopAllAudio();
}

void TutorialApplication::at_deadPanel_callback(ViRus::HitButton * h)
{
	app->drawMenu();
	h->destroy();
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
