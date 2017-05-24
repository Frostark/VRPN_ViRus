#pragma once

#include <list>
#include "BaseApplication.hpp"
#include <cmath>
#include "OgreSprites.hpp"

namespace ViRus
{
	//2D screen indicator, direction of damage on the XZ plane
	class DamageIndicator
	{
	private:

		static constexpr float DEF_TTL = 1.0f;//Default TTL for a damage indicator

	private:

		std::list< std::pair<float, float> > damages;//List of angles of attacks on the XZ plane, and their TTL
		float ttl;//TTL for a new a damage indicator
		float angle;//Current angle that the player is looking at

	public:

		DamageIndicator(float ittl = DEF_TTL)
			:damages(), ttl(ittl), angle(0.0f)
		{}

	public:

		//Add a damage indicator
		void add_damage(float iangle);

		//TTL
		void update_ttl(float delta_time);

		//Update the player's angle
		void update_angle(float iangle)
		{
			angle = iangle;
		}

		//Draw to screen
		void draw(Ogre2dManager &mgr);
	};

	//2D radar
	class Radar
	{
		private:

			Ogre::Vector3 player_pos;
			Ogre::Quaternion player_rot;

			Ogre::Vector2 radar_pos;

		public:

			Radar()
			:player_pos(), player_rot(), radar_pos(-0.775, -0.5)
			{}

		public:

			void update_player(Ogre::Vector3 ipos, Ogre::Quaternion irot)
			{
				player_pos = ipos;
				player_rot = irot;
			}

			void draw_radar(Ogre2dManager &mgr);

			void draw_enemy(Ogre2dManager &mgr, Ogre::Vector3 pos);
	};
}