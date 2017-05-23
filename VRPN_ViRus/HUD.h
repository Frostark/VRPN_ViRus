#pragma once

#include <list>
#include "BaseApplication.hpp"
#include <cmath>

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
		void draw();
	};
}