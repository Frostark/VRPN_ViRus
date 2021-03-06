#include "Gameplay.hpp"

namespace ViRus
{
	//Init
	Ogre::SceneManager *Gun::ptr_scn_mgr=nullptr;//Scene manager
	int Gun::numBullets=0;
	HitMap *Gun::hitmap=nullptr;
	OgreBulletDynamics::DynamicsWorld *Gun::mWorld=nullptr; // OgreBullet World

	Ogre::SceneManager *Spawner::ptr_scn_mgr=nullptr;//Scene manager
	int Spawner::total_spawned=0;//Total spawned enemies
	int Spawner::pickups_spawned = 0;//Total pickups spawned
	HitMap *Spawner::hitmap = nullptr;//Hittable container
	OgreBulletDynamics::DynamicsWorld *Spawner::mWorld=nullptr; // OgreBullet World
	std::default_random_engine Spawner::re;//Random engine

	Ogre::SceneManager *Menu::ptr_scn_mgr=nullptr;//Scene manager
	HitMap *Menu::hitmap=nullptr;//Hittable container
	OgreBulletDynamics::DynamicsWorld *Menu::mWorld=nullptr; // OgreBullet World
	int Menu::nButtons=0; // Number of buttons in the menu

	bool Gun::fire()
	{
		if (!delta_time)
		{
			if (audioEnabled)
				sound_mgr->playAudio(audio_fire_gun, true);

			Ogre::Vector3 from = barrel->getPosition();
			Ogre::Vector3 dir = barrel->getOrientation() * Ogre::Vector3(0, 0, -1);
			from += dir*barrelSize;
			
			// Create an ordinary, Ogre mesh with texture
			Ogre::Entity *bullet = ptr_scn_mgr->createEntity("Bullet" + Ogre::StringConverter::toString(numBullets), mesh_name);
			bullet->setCastShadows(true);
			Ogre::SceneNode *bulletNode = ptr_scn_mgr->getRootSceneNode()->createChildSceneNode();
			Ogre::SceneNode *bulletEntNode = bulletNode->createChildSceneNode();
			bulletEntNode->attachObject(bullet);

			// We need the bounding box of the entity to be able to set the size of the Bullet shape
			Ogre::AxisAlignedBox bulletBoundingBox = bullet->getBoundingBox();

			// Size of the Bullet shape, a box
			Ogre::Vector3 bulletShapeSize = Ogre::Vector3::ZERO;
			bulletShapeSize = bulletBoundingBox.getSize();
			bulletShapeSize /= 2.0f; // Only the half needed
			bulletShapeSize *= scale; // Bullet margin is a bit bigger so we need a smaller size

			bulletNode->scale(Ogre::Vector3(scale, scale, scale));
			bulletEntNode->translate(Ogre::Vector3(0, -bulletShapeSize.y, 2*bulletShapeSize.z),Ogre::Node::TS_WORLD);
			bulletNode->setOrientation(barrel->_getDerivedOrientation());
			bulletNode->pitch(Ogre::Degree(-90));

			Ogre::Quaternion rot = bulletNode->getOrientation();
			//Ogre::Vector3 pos = from+Ogre::Vector3(0,bulletShapeSize.y,0);

			// After that create the Bullet shape with the calculated size
			OgreBulletCollisions::BoxCollisionShape *bulletShape = new OgreBulletCollisions::BoxCollisionShape(bulletShapeSize);

			// and the Bullet rigid body
			OgreBulletDynamics::RigidBody *bulletBody = new OgreBulletDynamics::RigidBody("defaultBoxRigid" + Ogre::StringConverter::toString(numBullets), mWorld, ColliderType::HERO, ColliderType::ENEMY|ColliderType::OBSTACLE);
			bulletBody->setShape(bulletNode, bulletShape,
				0.6f, // dynamic body restitution
				0.6f, // dynamic body friction
				mass, // dynamic bodymass
				from, // starting position of the shape
				rot
			); // orientation of the shape
			bulletBody->setLinearVelocity(dir.normalisedCopy() * speed); // shooting speed

			bulletBody->getBulletRigidBody()->setAngularFactor(btVector3(0, 0, 0));
			numBullets++;

			// Push the created objects to the deques

			ViRus::HitProjectile *bulletHittable = new ViRus::HitProjectile(bulletBody, bulletShape, bulletNode, ViRus::TeamType::HERO, dmg);
			bulletHittable->set_callback(ptr_callback);
			bullets.push_back(bulletHittable);
			hitmap->add_hittable(*bulletBody->getBulletObject(), *bulletHittable);

			delta_time = max_delta;
			return true;
		}

		return false;
	}
	void Gun::refresh(double delta)
	{
		delta_time -= delta;
		if (delta_time < 0)
			delta_time = 0;

		for (HitProjectile *bullet : bullets)
			bullet->deltaTime(delta);
	}

	void Gun::callback(Hittable * h)
	{
		HitProjectile *bullet = static_cast<HitProjectile *>(h);

		bullets.remove(bullet);
	}

	void Gun::set_callback(void(*iptr_callback)(Hittable *))
	{
		ptr_callback = iptr_callback;
	}

	void Spawner::spawn()
	{
		currentWaitTime = maxWaitTime;

		std::uniform_real_distribution<> random_angle(0, Ogre::Degree(360).valueRadians());
		double angle = random_angle(re);

		Ogre::Vector2 vector(std::cos(angle), std::sin(angle));
		vector *= spawn_radius;

		Ogre::Entity *enemy = ptr_scn_mgr->createEntity("Enemy" + Ogre::StringConverter::toString(total_spawned), mesh_name);
		enemy->setCastShadows(true);
		Ogre::SceneNode *enemyNode = ptr_scn_mgr->getRootSceneNode()->createChildSceneNode();
		Ogre::SceneNode *enemyMeshNode = enemyNode->createChildSceneNode();
		enemyMeshNode->attachObject(enemy);

		enemyNode->scale(scale, scale, scale);
		Ogre::Vector3 enemySize = enemy->getBoundingBox().getSize();
		enemySize *= 0.5*scale;

		enemyMeshNode->translate(Ogre::Vector3(0, -enemySize.y, 0), Ogre::Node::TS_WORLD);

		Ogre::Vector3 position(vector.x, enemySize.y, vector.y);
		enemyNode->translate(position);


		OgreBulletCollisions::CylinderCollisionShape *cylinder = new OgreBulletCollisions::CylinderCollisionShape(enemySize, Ogre::Vector3::UNIT_Y);
		OgreBulletDynamics::RigidBody *body = new OgreBulletDynamics::RigidBody("EnemyBody" + Ogre::StringConverter::toString(total_spawned), mWorld, ColliderType::ENEMY, ColliderType::ENEMY | ColliderType::HERO | ColliderType::OBSTACLE);
		body->setShape(enemyNode, cylinder, restitution, friction, mass, position);
		body->getBulletRigidBody()->setAngularFactor(btVector3(0, 0, 0));

		HitCharAttack *ptr = new HitCharAttack(body, cylinder, enemyNode, team, health, dmg, timeAttack, vel);

		ptr->set_callback(ptr_callback);
		ptr->set_entity(enemy);
		enemies.push_back(ptr);

		hitmap->add_hittable(*body->getBulletObject(), *ptr);

		n_enemies++;
		total_spawned++;
	}
	void Spawner::kill_all()
	{
		for (HitCharAttack *ptr : enemies)
		{
			hitmap->delete_hittable(*ptr);
		}

		for (HitPickup *ptr : pickups)
		{
			hitmap->delete_hittable(*ptr);
		}
	}
	void Spawner::chase(HitCharacter &h)
	{
		for (HitCharAttack *ptr : enemies)
		{
			ptr->chase(h);
		}
	}

	void Spawner::deltaTime(double itime)
	{
		currentWaitTime -= itime;
		if (currentWaitTime < 0)
			currentWaitTime = 0;

		for (HitCharAttack * ptr : enemies)
			ptr->deltaTime(itime);

		for (HitPickup *ptr : pickups)
			ptr->delta_time(itime);
	}
	void Spawner::spawn_medkit(Ogre::Vector3 pos)
	{
		std::uniform_real_distribution<> prob;
		if (prob(re) < probMedkit)
		{
			Ogre::Entity *entity = ptr_scn_mgr->createEntity(medkit_mesh_name);
			entity->setCastShadows(true);
			Ogre::SceneNode *node = ptr_scn_mgr->getRootSceneNode()->createChildSceneNode();
			Ogre::SceneNode *entNode = node->createChildSceneNode();
			entNode->attachObject(entity);

			constexpr float scale = 1;

			node->scale(scale, scale, scale);
			Ogre::Vector3 size = entity->getBoundingBox().getSize();
			size *= 0.5*scale;

			entNode->translate(Ogre::Vector3(-size.x,-size.y,size.z), Ogre::Node::TS_WORLD);

			Ogre::Vector3 position = pos;

			OgreBulletCollisions::BoxCollisionShape *box = new OgreBulletCollisions::BoxCollisionShape(size);
			OgreBulletDynamics::RigidBody *body = new OgreBulletDynamics::RigidBody("medkit"+ Ogre::StringConverter::toString(pickups_spawned), mWorld, ColliderType::POWERUP, ColliderType::HERO|ColliderType::OBSTACLE);
			body->setShape(node, box, restitution, friction, mass, position);
			body->getBulletRigidBody()->setAngularFactor(btVector3(0, 0, 0));

			HitMedkit *hittable = new HitMedkit(body, box, node);

			hittable->set_callback(ptr_pickup_callback);
			pickups.push_back(hittable);

			hitmap->add_hittable(*body->getBulletObject(), *hittable);
			pickups_spawned++;
		}
	}
	void Spawner::draw_radar(Radar & rdr, Ogre2dManager & mgr)
	{
		for (HitCharAttack *ptr : enemies)
		{
			Ogre::Vector3 pos;
			if (ptr->get_position(pos))
			{
				rdr.draw_enemy(mgr, pos);
			}
		}
	}

	void Menu::drawBasic()
	{
		// Create an Menu Entity 
		ogreMenu = ptr_scn_mgr->createEntity("Menu", menu_mesh);
		// Create a Menu and attach the Entity to it
		Ogre::SceneNode* menuNode = ptr_scn_mgr->getRootSceneNode()->createChildSceneNode();

		menuNode->scale(Ogre::Vector3(0.1, 0.1, 0.1));
		menuNode->translate(Ogre::Vector3(8, 1.1, -4));
		menuNode->attachObject(ogreMenu);
		menuNode->rotate(Vector3::NEGATIVE_UNIT_Y, Degree(90));

	}

	void Menu::show() 
	{ 
		ogreMenu->setVisible(true); 
	}

	void Menu::addButton(std::string imesh_name, Ogre::Vector3 ipos, void(*iat_button) (HitButton *))
	{
		// Create an QuitButton Entity 
		Ogre::Entity* ogreButton = ptr_scn_mgr->createEntity("Button" + Ogre::StringConverter::toString(nButtons), imesh_name);
		Ogre::SceneNode* buttonNode = ptr_scn_mgr->getRootSceneNode()->createChildSceneNode();
		Ogre::SceneNode* entNode = buttonNode->createChildSceneNode();
		entNode->attachObject(ogreButton);

		buttonNode->scale(Ogre::Vector3(0.1, 0.1, 0.1));

		Ogre::Vector3 size = ogreButton->getBoundingBox().getSize()*0.5*0.1;

		entNode->translate(Ogre::Vector3(-size.x, -size.y, 0), Ogre::Node::TS_WORLD);

		buttonNode->rotate(Vector3::NEGATIVE_UNIT_Y, Degree(90));
		buttonNode->translate(ipos);

		Ogre::Vector3 position = buttonNode->getPosition();
		Ogre::Quaternion rot = buttonNode->getOrientation();

		OgreBulletCollisions::BoxCollisionShape *box = new OgreBulletCollisions::BoxCollisionShape(size);
		OgreBulletDynamics::RigidBody *body = new OgreBulletDynamics::RigidBody("Body" + Ogre::StringConverter::toString(nButtons), mWorld, ViRus::ColliderType::OBSTACLE, ViRus::ColliderType::HERO);
		body->setStaticShape(buttonNode, box, 1, 1, position, rot);
		body->setKinematicObject(true);
		body->disableDeactivation();

		ViRus::HitButton *ptr_button = new ViRus::HitButton(body, box, buttonNode);

		ptr_button->set_at_button(iat_button);
		
		hitmap->add_hittable(*body->getBulletObject(), *ptr_button);

		buttons.push_back(ptr_button);
		nButtons++;
	}

	void Menu::hide()
	{
		for (HitButton *ptr : buttons)
		{
			hitmap->delete_hittable(*ptr);
		}

		buttons.clear();
		
		// Delete Menu Node here
		if (ogreMenu)
		{
			ogreMenu->setVisible(false);
		}
	}
	void Menu::showPanel(std::string imesh_name)
	{
		Ogre::Entity* ogrePanel = ptr_scn_mgr->createEntity("Panel", imesh_name);
		Ogre::SceneNode* panelNode = ptr_scn_mgr->getRootSceneNode()->createChildSceneNode();

		panelNode->scale(Ogre::Vector3(0.1, 0.1, 0.1));
		panelNode->translate(Ogre::Vector3(6, 1.15, -6));
		panelNode->attachObject(ogrePanel);
		panelNode->rotate(Vector3::NEGATIVE_UNIT_Y, Degree(90));
	}
}