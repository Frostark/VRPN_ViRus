#ifndef _VIRUS_GAMEPLAY_
#define _VIRUS_GAMEPLAY_

#include "OgreBulletDynamicsRigidBody.h" // for OgreBullet
#include "Shapes/OgreBulletCollisionsBoxShape.h" // for boxes

#include "Physics.hpp"
#include "BaseApplication.hpp"
#include <string>

namespace ViRus
{
	class Gun;//Basic gun

			  //Basic gun
	class Gun
	{
	private:
		static constexpr double DEF_SPEED = 100;//Default speed of the bullet
		static constexpr double DEF_MAX_TIME = 0.25;//Default time between shots
		static constexpr double DEF_DMG = 10;//Default damage
		static constexpr double DEF_SCALE = 0.01;//Default scale of the bullet
		static constexpr double DEF_MASS = 100;//Default mass

	public:
		static Ogre::SceneManager *ptr_scn_mgr;//Scene manager
		static int numBullets;
		static HitMap *hitmap;
		static OgreBulletDynamics::DynamicsWorld *mWorld; // OgreBullet World

	protected:
		Ogre::SceneNode *barrel;//Point at where the gun fires
		std::string mesh_name;//Number of the mesh of the bullet
		double scale;//Scale of the entity
		double dmg;//Damage of the bullet
		double speed;//Speed of the bullet
		double mass;//Mass of the bullet
		double delta_time, max_delta;//Current time since last shot

	public:

		//Complete constructor
		Gun(Ogre::SceneNode *ibarrel, std::string imesh_name, double iscale = DEF_SCALE, double idmg = DEF_DMG, double ispeed = DEF_SPEED, double imass = DEF_MASS, double imax_delta = DEF_MAX_TIME)
			:barrel(ibarrel), mesh_name(imesh_name), scale(iscale), dmg(idmg), speed(ispeed), mass(imass), delta_time(0), max_delta(imax_delta)
		{}

		//Virtual distructor
		virtual ~Gun() {}

	public:

		virtual bool fire();//Fire the gun if possible, return true if it fired

		void refresh(double delta);//Refresh the gun's delta

	};
}

#endif // !_VIRUS_GAMEPLAY_

