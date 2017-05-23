#include "HUD.h"

namespace ViRus
{
	void DamageIndicator::add_damage(float iangle)
	{
		damages.emplace_back(iangle, ttl);
	}
	void DamageIndicator::update_ttl(float delta_time)
	{
		for (auto it = damages.begin(); it != damages.end();)
		{
			std::pair<float, float> &ref = *it;
			ref.second -= delta_time;
			if (ref.second <= 0)//Delete when TTL hits zero
			{
				it = damages.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	void DamageIndicator::draw()
	{
		for (std::pair<float, float> &ref : damages)
		{
			float display_angle = ref.first - angle;
			Ogre::Vector2 dir(std::cos(display_angle), std::sin(display_angle));

			//TODO: draw
		}
	}
}