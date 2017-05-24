#ifndef _VIRUS_PHYSICS_
#define _VIRUS_PHYSICS_


#include "OgreBulletDynamicsRigidBody.h" // for OgreBullet
#include "Shapes/OgreBulletCollisionsStaticPlaneShape.h" // for static planes
#include "Shapes/OgreBulletCollisionsBoxShape.h" // for boxes
#include "BaseApplication.hpp"
#include <iostream>
#include <fstream>

#include <map>
#include <set>

#include "HUD.h"

namespace ViRus
{

	//Types

	//Teams
	enum class TeamType : char
	{
		START=0,
		HERO,
		ENEMY,
		END
	};

	enum ColliderType : short int
	{
		NOTHING = 0,
		HERO = 1,
		ENEMY = 2,
		POWERUP = 4,
		OBSTACLE = 8
	};

	//Prototypes
	void destroy_node(Ogre::SceneNode *node);

	//Class declarations
	class Hittable;//Object that can be hit
	class HitPickup;//Hittable of a pickup
	class HitMedkit;//Hittable of a pickup
	class HitObstacle;//Hittable of a static obstacle
	class Teamable;//Object that has a team
	class HitProjectile;//Projectile that can hit objects
	class HitCharacter;//Character that can be hit
	class HitCharAttack;//Character that hits others upon contact
	class HitPlayer;//Character of the player
	class HitMap;//Map that goes from the physics object to the hittable object
	class HitButton;//Hittable of a button

	//Class definitions

	//Object that can be hit
	class Hittable
	{
		public:
			//Pointer to the scene manager, needed to clean up hittables. Needs to be set to the current scene manager before using HitMap
			static Ogre::SceneManager *ptr_scn_mgr;

			friend class HitMap;

		protected:
				//Pointers to clean up at deletion, callback
				OgreBulletDynamics::RigidBody *body;
				OgreBulletCollisions::CollisionShape *shape;
				Ogre::SceneNode *scene;
				void(*callback) (Hittable *) = nullptr;


			public:
				//Complete constructor
				Hittable(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene)
				:body(ibody),shape(ishape),scene(iscene)
				{}

				//Destructor, calls callback if set and performs clean up
				virtual ~Hittable();

			public:

				//Hit another hittable
				virtual void hit(Hittable &h)
				{}

				//Return true if this hittable is done and can be removed from the map
				virtual bool finished()
				{
					return false;
				}

				//Set the callback to be called before deletion
				void set_callback(void(*icallback) (Hittable *))
				{
					callback = icallback;
				}
	};


	//Hittable of a static obstacle
	class HitObstacle : public Hittable
	{
		public:

			//Complete constructor
			HitObstacle(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene)
			:Hittable(ibody, ishape, iscene)
			{}

			//Destructor
			~HitObstacle() {}
	};

	//Hittable of a pickup
	class HitPickup : public Hittable
	{
		protected:

			bool isUsed;
			float ttl = 5;

		protected:

			//Complete constructor
			HitPickup(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene)
			:Hittable(ibody,ishape,iscene),isUsed(false)
			{}

			//Destructor
			virtual ~HitPickup() {};

		public:

			virtual void effect(HitPlayer *player) = 0;

			virtual void delta_time(double itime) = 0;

			bool finished()
			{
				return isUsed;
			}

			//Hit another hittable
			void hit(Hittable &h);
	};

	//Hittable of a pickup
	class HitMedkit: public HitPickup
	{
		public:

			//Complete constructor
			HitMedkit(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene)
				:HitPickup(ibody, ishape, iscene)
			{}

			//Destructor
			virtual ~HitMedkit() {};

		public:

			void effect(HitPlayer *player);
			void delta_time(double itime);

	};

	//Object that has a team
	class Teamable
	{
		protected:
			TeamType team;//Team of this object

		protected:

			//Complete constructor
			Teamable(TeamType iteam)
			:team(iteam)
			{}

		public:

			//Destructor
			virtual ~Teamable() {}

		public:

			//Get this team
			virtual TeamType get_team() const
			{
				return team;
			}

			//Check if this team attacks the other
			virtual bool attack_team(const Teamable &iteam) const;

	};

	//Projectile
	class HitProjectile : public Hittable, public Teamable
	{
		private:
			bool isFinished = false;//Bullet has hit something, and despawns
			int dmg;//Damage that this bullet deals upon hitting a character
			float ttl;//Time to live

		public:

			//Complete constructor
			HitProjectile(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene, TeamType iteam, int idmg=0, float ittl=1.0f)
			:Hittable(ibody,ishape,iscene), Teamable(iteam), dmg(idmg), ttl(ittl)
			{}

			//Virtual destructor
			virtual ~HitProjectile() {}

		public:

			//Hit another hittable
			virtual void hit(Hittable &h);

			//Return true if this hittable is done and can be removed from the map
			virtual bool finished()
			{
				return isFinished;
			}

			//Update the ttl with the given times
			void deltaTime(double itime)
			{
				ttl -= itime;
				if (ttl <= 0)
					isFinished = true;
			}

	};

	//Character that can be hit
	class HitCharacter : public Hittable, public Teamable
	{
		protected:
			int health;//Health that his character has (below 0, character is dead

		public:

			//Complete destructor
			HitCharacter(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene, TeamType iteam, int ihealth)
			:Hittable(ibody,ishape,iscene),Teamable(iteam), health(ihealth)
			{}

			virtual ~HitCharacter() {}

		public:

			//Hit another hittable
			virtual void hit(Hittable &h) {}

			//Return true if this hittable is done and can be removed from the map
			virtual bool finished()
			{
				return health <= 0;
			}

			//Take damage
			virtual void takeDamage(int idmg)
			{
				health -= idmg;
			}

			//Get the position of scene. Return true if the vector was changed, false if there was no scene node
			bool get_position(Ogre::Vector3 &pos) const;
	};


	enum class CharacterAnimState : int
	{
		START=0,
		WALK,
		ATTACK,
		DEATH,
		END
	};

	//Character that hits others upon contact
	class HitCharAttack : public HitCharacter
	{
		protected:
			static constexpr double DEF_VEL = 4;//Default velocity of the character
			constexpr static double DEF_ATTACK_TIME = 2.0;//Default time between attacks


		protected:

			int dmg;//Damage that the enemy performs at contact
			double deltaAttack = 0.0, timeAttack;//Time elapsed between attacks, actual and max
			double vel;//Velocity of the character
			CharacterAnimState anim;
			Ogre::AnimationState *anim_controller;
			Ogre::Entity *entity;

		public:

			//Complete constructor
			HitCharAttack(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene, TeamType iteam, int ihealth, int idmg, double itimeAttack = DEF_ATTACK_TIME, double ivel = DEF_VEL)
			:HitCharacter(ibody, ishape, iscene, iteam, ihealth), dmg(idmg), timeAttack(itimeAttack), vel(ivel), anim(CharacterAnimState::START),anim_controller(nullptr),entity(nullptr)
			{
			}

			//Virtual destructor
			virtual ~HitCharAttack() {}

		public:

			//Hit another hittable
			virtual void hit(Hittable &h);

		public:

			//Take damage
			void takeDamage(int idmg)
			{
				HitCharacter::takeDamage(idmg);

				if (health > 0)
				{
					sound_mgr->playAudio(audio_enemy_hit, true);
				}
				else
				{
					sound_mgr->playAudio(audio_enemy_dead, true);
				}
			}

			//Return true if this hittable is done and can be removed from the map, play death animation before dying
			virtual bool finished();

			//Update the time between attacks with the given elapsed time
			void deltaTime(double itime);

			//Go to a point
			void go_point(Ogre::Vector3 pos);

			//Chase a character
			void chase(const HitCharacter &h);

			//Get the entity
			Ogre::Entity *get_entity();

			//Set the entity
			void set_entity(Ogre::Entity *ientity);

	};

	//Character of the player
	class HitPlayer : public HitCharacter
	{
		private:
			bool(*at_death) (HitPlayer *);//Call this function on finished(), when the player dies. Return the value that this callback returns
			int init_health;//Initial health

			DamageIndicator &di;//Reference to damage indicator
			Radar &rdr;//Reference to radar

		public:

			//Complete constructor
			HitPlayer(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene, TeamType iteam, int ihealth, DamageIndicator &idi, Radar &irdr)
			:HitCharacter(ibody,ishape,iscene,iteam,ihealth), at_death(nullptr), init_health(ihealth), di(idi),rdr(irdr)
			{
				update_di();
				update_rdr();
			}

		public:

			//Set at_death callback
			void set_at_death(bool(*iat_death) (HitPlayer *))
			{
				at_death = iat_death;
			}

			//Return true if this hittable is done and can be removed from the map
			bool finished()
			{
				if (HitCharacter::finished())
				{
					if (at_death)
						return at_death(this);
					else
						return true;
				}
				return false;
			}

			//Take damage
			void takeDamage(int idmg)
			{
				HitCharacter::takeDamage(idmg);

				if (health > 0)
				{
					sound_mgr->playAudio(audio_player_hit, true);
				}
				else
				{
					sound_mgr->playAudio(audio_player_dead, true);
				}
			}

			void directional_damage(Ogre::Vector3 pos);

			void revive()
			{
				health = init_health;
			}

			int get_health()
			{
				if (health < 0)
					return 0;
				return health;
			}

			void update_di();//Update the damage indicator

			void update_rdr();//Update the radar
	};



	//Map that goes from the physics object to the hittable object
	class HitMap
	{
		private:

			std::map<btCollisionObject*, Hittable *> hittables;//Map that goes from the bullet object pointer to the hittable pointer
			std::set<btCollisionObject *> clean_up;//Queue of objects to delete

		public:

			~HitMap();

		public:

			//Add a hittable
			void add_hittable(btCollisionObject &c, Hittable &h);

			//Handle collision
			void handle_collision(btCollisionObject *a, btCollisionObject *b);

			//Delete pointers queued for destroctions
			void clean_queued();

			//Destroy a hittable
			void delete_hittable(Hittable &h);

		public:

			//Delete all hittables
			void clear_all();
	};

	//Hittable of a button
	class HitButton : public Hittable
	{
		private: 
			void(*at_button) (HitButton *);//Call this function, when the button is hit.

		public:

			//Complete constructor
			HitButton(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene)
				:Hittable(ibody, ishape, iscene)
			{}

			//Destructor
			~HitButton() {}

			//Hit another hittable
			virtual void hit(Hittable &h) 
			{
				HitProjectile *ptr = dynamic_cast<HitProjectile*>(&h);
				if (ptr)
				{
					at_button(this);
				}
			}

			//Set the callback to be called when the bulled hits the button
			void set_at_button(void(*iat_button) (HitButton *))
			{
				at_button = iat_button;
			}
	};

}

#endif // !_VIRUS_PHYSICS_
