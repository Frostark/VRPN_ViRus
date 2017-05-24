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
	void Radar::draw_radar(Ogre2dManager &mgr)
	{
		static constexpr float RADAR_SIZE = 0.05;
		static constexpr float ASPECT_RATIO = 16.0 / 9.0;

		mgr.spriteBltFull("radar.png", radar_pos.x - RADAR_SIZE, radar_pos.y + RADAR_SIZE * ASPECT_RATIO, radar_pos.x + RADAR_SIZE, radar_pos.y - RADAR_SIZE*ASPECT_RATIO);
	}
	void Radar::draw_enemy(Ogre2dManager &mgr, Ogre::Vector3 pos)
	{
		static constexpr float DOT_SIZE = 0.01;
		static constexpr float COEF_DISTANCE_CENTER = 0.1;
		static constexpr float MAX_DISTANCE = 1;
		static constexpr float ASPECT_RATIO = 16.0 / 9.0;

		Ogre::Vector3 dir(pos - player_pos);

		Ogre::Vector2 dir_2d(dir.x, -dir.z);

		float dir_angle = Ogre::Vector2::UNIT_X.angleTo(dir_2d).valueRadians();

		Ogre::Vector3 front = player_rot * Ogre::Vector3::NEGATIVE_UNIT_Z;
		Ogre::Vector2 front_2d(front.x, -front.z);
		float front_angle = Ogre::Vector2::UNIT_Y.angleTo(front_2d).valueRadians();

		float display_angle = dir_angle - front_angle;

		float size = dir_2d.length() * COEF_DISTANCE_CENTER;
		Ogre::Vector2 display_vector(std::cos(display_angle), std::sin(display_angle));
		display_vector *= size;

		if (display_vector.length() <= MAX_DISTANCE)
		{
			display_vector += radar_pos;

			mgr.spriteBltFull("enemyDot.png", display_vector.x - DOT_SIZE, display_vector.y + DOT_SIZE * ASPECT_RATIO, display_vector.x + DOT_SIZE, display_vector.y - DOT_SIZE*ASPECT_RATIO);
		}
	}
}