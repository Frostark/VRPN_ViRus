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
	HitMap *Spawner::hitmap = nullptr;//Hittable container
	OgreBulletDynamics::DynamicsWorld *Spawner::mWorld=nullptr; // OgreBullet World
	std::default_random_engine Spawner::re;//Random engine

	bool Gun::fire()
	{
		if (!delta_time)
		{
			Ogre::Vector3 from = barrel->getPosition();
			Ogre::Vector3 dir = barrel->getOrientation() * Ogre::Vector3(0, 0, -1);
			
			// Create an ordinary, Ogre mesh with texture
			Ogre::Entity *bullet = ptr_scn_mgr->createEntity("Bullet" + Ogre::StringConverter::toString(numBullets), mesh_name);
			bullet->setCastShadows(true);
			Ogre::SceneNode *bulletNode = ptr_scn_mgr->getRootSceneNode()->createChildSceneNode();
			bulletNode->attachObject(bullet);

			// We need the bounding box of the entity to be able to set the size of the Bullet shape
			Ogre::AxisAlignedBox bulletBoundingBox = bullet->getBoundingBox();

			// Size of the Bullet shape, a box
			Ogre::Vector3 bulletShapeSize = Ogre::Vector3::ZERO;
			bulletShapeSize = bulletBoundingBox.getSize();
			bulletShapeSize /= 2.0f; // Only the half needed
			bulletShapeSize *= scale; // Bullet margin is a bit bigger so we need a smaller size

			bulletNode->scale(Ogre::Vector3(scale, scale, scale));

			// After that create the Bullet shape with the calculated size
			OgreBulletCollisions::BoxCollisionShape *bulletShape = new OgreBulletCollisions::BoxCollisionShape(bulletShapeSize);

			// and the Bullet rigid body
			OgreBulletDynamics::RigidBody *bulletBody = new OgreBulletDynamics::RigidBody("defaultBoxRigid" + Ogre::StringConverter::toString(numBullets), mWorld);
			bulletBody->setShape(bulletNode, bulletShape,
				0.6f, // dynamic body restitution
				0.6f, // dynamic body friction
				mass, // dynamic bodymass
				from, // starting position of the shape
				Ogre::Quaternion(0, 0, 0, 1)
			); // orientation of the shape
			bulletBody->setLinearVelocity(dir.normalisedCopy() * speed); // shooting speed

			bulletBody->getBulletRigidBody()->setAngularFactor(btVector3(0, 0, 0));
			numBullets++;

			// Push the created objects to the deques

			ViRus::Hittable *bulletHittable = new ViRus::HitProjectile(bulletBody, bulletShape, bulletNode, ViRus::TeamType::HERO, dmg);
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
	}

	void Spawner::spawn()
	{
		std::uniform_real_distribution<> random_angle(0, Ogre::Degree(360).valueRadians());
		double angle = random_angle(re);

		Ogre::Vector2 vector(std::cos(angle), std::sin(angle));
		vector *= spawn_radius;

		Ogre::Entity *enemy = ptr_scn_mgr->createEntity("Enemy" + Ogre::StringConverter::toString(total_spawned), mesh_name);
		enemy->setCastShadows(true);
		Ogre::SceneNode *enemyNode = ptr_scn_mgr->getRootSceneNode()->createChildSceneNode();
		enemyNode->attachObject(enemy);

		enemyNode->scale(scale, scale, scale);
		Ogre::Vector3 enemySize = enemy->getBoundingBox().getSize();
		enemySize /= 2.0*scale;

		Ogre::Vector3 position(vector.x, enemySize.y, vector.y);


		OgreBulletCollisions::CylinderCollisionShape *cylinder = new OgreBulletCollisions::CylinderCollisionShape(enemySize, Ogre::Vector3::UNIT_Y);
		OgreBulletDynamics::RigidBody *body = new OgreBulletDynamics::RigidBody("EnemyBody" + Ogre::StringConverter::toString(total_spawned), mWorld);
		body->setShape(enemyNode, cylinder, restitution, friction, mass, position);
		body->getBulletRigidBody()->setAngularFactor(btVector3(0, 0, 0));

		enemies.emplace_back(body, cylinder, enemyNode, team, health, dmg, timeAttack, vel);

		HitCharAttack &ref_hit = enemies.back();

		hitmap->add_hittable(*body->getBulletObject(), ref_hit);
	}
	void Spawner::kill_all()
	{
		for (HitCharAttack &ref : enemies)
		{
			hitmap->delete_hittable(ref);
		}

		enemies.clear();
	}
}