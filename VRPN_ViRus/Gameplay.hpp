#ifndef _VIRUS_GAMEPLAY_
#define _VIRUS_GAMEPLAY_

#include "OgreBulletDynamicsRigidBody.h" // for OgreBullet
#include "Shapes/OgreBulletCollisionsBoxShape.h" // for boxes
#include "Shapes/OgreBulletCollisionsCylinderShape.h"

#include "Physics.hpp"
#include "BaseApplication.hpp"
#include <string>
#include <list>
#include <random>
#include <cmath>
#include <utility>
#include "Physics.hpp"
#include "HUD.h"

namespace ViRus
{
	class Gun;//Basic gun

	class Spawner;//Enemy spawner

	class DamageIndicator;//2D screen indicator, direction of damage on the XZ plane

	class Menu; // Draw Main menu

			  //Basic gun
	class Gun
	{
	private:
		static constexpr double DEF_SPEED = 100;//Default speed of the bullet
		static constexpr double DEF_MAX_TIME = 0.25;//Default time between shots
		static constexpr double DEF_DMG = 26;//Default damage
		static constexpr double DEF_SCALE = 0.01;//Default scale of the bullet
		static constexpr double DEF_MASS = 100;//Default mass

	public:
		static Ogre::SceneManager *ptr_scn_mgr;//Scene manager
		static int numBullets;
		static HitMap *hitmap;
		static OgreBulletDynamics::DynamicsWorld *mWorld; // OgreBullet World

	protected:
		Ogre::SceneNode *barrel;//Point at where the gun fires
		float barrelSize;//Size of the gun's barrel
		std::string mesh_name;//Number of the mesh of the bullet
		double scale;//Scale of the entity
		double dmg;//Damage of the bullet
		double speed;//Speed of the bullet
		double mass;//Mass of the bullet
		double delta_time, max_delta;//Current time since last shot
		std::list<HitProjectile *> bullets;//List of active bullets
		void(*ptr_callback) (Hittable *) = nullptr;

	public:

		//Complete constructor
		Gun(Ogre::SceneNode *ibarrel, float ibarrelSize, std::string imesh_name, double iscale = DEF_SCALE, double idmg = DEF_DMG, double ispeed = DEF_SPEED, double imass = DEF_MASS, double imax_delta = DEF_MAX_TIME)
			:barrel(ibarrel), barrelSize(ibarrelSize), mesh_name(imesh_name), scale(iscale), dmg(idmg), speed(ispeed), mass(imass), delta_time(0), max_delta(imax_delta), bullets()
		{}

		//Virtual distructor
		virtual ~Gun() {}

	public:

		virtual bool fire();//Fire the gun if possible, return true if it fired

		void refresh(double delta);//Refresh the gun's delta

		void callback(Hittable *h);//Gun callback, remove the bullet

		void set_callback(void(*ptr_callback) (Hittable *));

	};

	//Enemy spawner
	class Spawner
	{
		public:


			static Ogre::SceneManager *ptr_scn_mgr;//Scene manager
			static int total_spawned;//Total spawned enemies
			static int pickups_spawned;//Total pickups spawned
			static HitMap *hitmap;//Hittable container
			static OgreBulletDynamics::DynamicsWorld *mWorld; // OgreBullet World
			static std::default_random_engine re;

		private:

			Ogre::Vector3 spawn_center;//Center of the spawner area
			float spawn_radius;//Distance to the center where enemies will spawn

			int n_enemies;//Current number of enemies from this spawner
			int max_enemies;//Max number of enemies from this spawner

			ViRus::TeamType team;//Team of the enemy
			int health;//Health of the enemy
			int dmg;//Damage the enemy does
			double timeAttack;//Seconds between enemy attacks
			double vel;//Enemy velocity

			std::string mesh_name;//Number of the mesh of the bullet
			double scale;//Scale of the entity

			float restitution, friction, mass;//OgreBullet parameters

			std::list<HitCharAttack *> enemies;//List of all spawned alive enemies
			void(*ptr_callback) (Hittable *) = nullptr;

			double currentWaitTime, maxWaitTime;

			double probMedkit;
			std::string medkit_mesh_name;


			std::list<HitPickup *> pickups;
			void(*ptr_pickup_callback) (Hittable *) = nullptr;

		public:

			//Complete constructor
			Spawner(Ogre::Vector3 ispawn_center, float ispawn_radius, int imax_enemies, ViRus::TeamType iteam, int ihealth, int idmg, double itimeAttack, double ivel, std::string imesh_name, double iscale, double irestitution, double ifriction, double imass, double imaxWaitTime, double iprobMedkit, std::string imedkit_mesh_name)
			:spawn_center(ispawn_center), spawn_radius(ispawn_radius), n_enemies(0), max_enemies(imax_enemies), team(iteam), health(ihealth), dmg(idmg), timeAttack(itimeAttack), vel(ivel), mesh_name(imesh_name), scale(iscale),restitution(irestitution),friction(ifriction),mass(imass),enemies(),ptr_callback(nullptr),currentWaitTime(0),maxWaitTime(imaxWaitTime),probMedkit(iprobMedkit),medkit_mesh_name(imedkit_mesh_name),pickups()
			{}

		public:

			void set_callback(void(*icallback) (Hittable *))
			{
				ptr_callback = icallback;
			}

			void set_pickup_callback(void(*iptr_pickup_callback) (Hittable *))
			{
				ptr_pickup_callback = iptr_pickup_callback;
			}

			bool need_spawn()
			{
				return (!currentWaitTime) && (n_enemies < max_enemies);
			}

			void callback(Hittable *h)
			{
				hitmap->delete_hittable(*h);
				n_enemies--;

				Ogre::Vector3 pos;
				if ((static_cast<HitCharAttack *>(h))->get_position(pos))
					spawn_medkit(pos);

				enemies.remove(static_cast<HitCharAttack *>(h));
			}

			void medkit_callback(Hittable *h)
			{
				pickups.remove(static_cast<HitPickup *>(h));
			}

			void spawn();

			void kill_all();

			void chase(HitCharacter &h);

			void deltaTime(double itime);

			void spawn_medkit(Ogre::Vector3 pos);

			void draw_radar(Radar &rdr, Ogre2dManager &mgr);
	};

	//Draw main menu
	class Menu
	{
		public:
			static Ogre::SceneManager *ptr_scn_mgr;//Scene manager
			static HitMap *hitmap;//Hittable container
			static OgreBulletDynamics::DynamicsWorld *mWorld; // OgreBullet World
			static int nButtons; // Number of buttons in the menu

		private:
			std::string menu_mesh; //Main menu mesh
			std::list<HitButton *> buttons; // Hittable buttons in the menu
			Ogre::Entity* ogreMenu;  // Basic menu (without buttons)

		public:

			// Complete constructor
			Menu(std::string imenu_mesh)
			: menu_mesh(imenu_mesh), ogreMenu()
			{
				drawBasic();
			}

			// Destructor
			~Menu() {}

		public:

			// Draw menu without buttons
			void drawBasic();

			// Show menu
			void show();
			
			// Add a new button
			void addButton(std::string imesh_name, Ogre::Vector3 ipos, void(*iat_button) (HitButton *));

			// Despawn the menu
			void hide();

			// Show panel
			void showPanel(std::string imesh_name);

	};
}

#endif // !_VIRUS_GAMEPLAY_

