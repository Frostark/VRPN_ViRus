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
	void DamageIndicator::draw(Ogre2dManager &mgr)
	{
		for (std::pair<float, float> &ref : damages)
		{
			float display_angle = ref.first - angle;
			Ogre::Vector2 dir(std::cos(display_angle),std::sin(display_angle));

			static constexpr float BALL_SIZE = 0.05;
			static constexpr float DISTANCE_CENTER = 0.75;
			static constexpr float ASPECT_RATIO = 16.0 / 9.0;

			dir *= DISTANCE_CENTER;

			mgr.spriteBltFull("damageIndicator.png", dir.x - BALL_SIZE, dir.y + BALL_SIZE*ASPECT_RATIO, dir.x + BALL_SIZE, dir.y - BALL_SIZE*ASPECT_RATIO);
		}
	}
}