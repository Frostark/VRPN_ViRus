#include "Physics.hpp"

namespace ViRus
{
	Ogre::SceneManager * Hittable::ptr_scn_mgr = nullptr;

	Hittable::~Hittable()
	{
		//Call the callback, is set
		if (callback)
			(callback)(this);


		//Delete the bullet objects
		delete body;
		delete shape;


		//Delete the scene node
		if (scene)
		{
			destroy_node(scene);

			scene->removeAndDestroyAllChildren();

			//Delete the node
			if (ptr_scn_mgr)
				ptr_scn_mgr->destroySceneNode(scene);
		}
	}

	void HitProjectile::hit(Hittable & h)
	{
		//Do not hit twice
		if (isFinished)
			return;

		//Check if you're hitting a character
		HitCharacter *ptr_hitcharacter = dynamic_cast<HitCharacter *>(&h);
		if (ptr_hitcharacter)
		{
			//Check if you're hitting an enemy
			if (attack_team(*ptr_hitcharacter))
			{
				//Perform damage
				ptr_hitcharacter->takeDamage(dmg);
				isFinished = true;
			}
			return;
		}

		//Check if you're hitting an obstacle
		HitObstacle *ptr_hitobstacle = dynamic_cast<HitObstacle *>(&h);
		if (ptr_hitobstacle)
		{
			//isFinished = true;
			return;
		}
	}

	HitMap::~HitMap()
	{
		clear_all();
	}

	void HitMap::add_hittable(btCollisionObject &c, Hittable &h)
	{
		hittables.insert(std::pair<btCollisionObject *, Hittable *>(&c, &h));
	}
	void HitMap::handle_collision(btCollisionObject * a, btCollisionObject * b)
	{

		//Check that none of the pointers are null
		if (a&&b)
		{
			Hittable *ptr_a = nullptr, *ptr_b = nullptr;

			//Get the hittables
			auto it_a = hittables.find(a);
			if (it_a != hittables.end())
				ptr_a = it_a->second;
			auto it_b = hittables.find(b);
			if (it_b != hittables.end())
				ptr_b = it_b->second;

			//Check that both hittables were found
			if (ptr_a&&ptr_b)
			{
				//Hit eachother
				ptr_a->hit(*ptr_b);
				ptr_b->hit(*ptr_a);

				//Check if either of them needs to be deleted
				if (ptr_a->finished())
				{
					clean_up.insert(a);
				}

				if (ptr_b->finished())
				{
					clean_up.insert(b);
				}
			}
		}
	}

	void HitMap::clean_queued()
	{
		for (btCollisionObject *obj : clean_up)
		{
			auto it = hittables.find(obj);
			if (it != hittables.end())
			{
				Hittable *hittable = it->second;
				delete hittable;
				hittables.erase(it);
			}
		}

		clean_up.clear();
	}

	void HitMap::delete_hittable(Hittable & h)
	{
		clean_up.insert(h.body->getBulletObject());
	}

	void HitMap::clear_all()
	{
		for (auto &it : hittables)
		{
			delete it.second;
		}

		hittables.clear();
	}

	bool Teamable::attack_team(const Teamable &iteam) const
	{
		bool rv = false;//Return value

		switch (get_team())//Team of this object
		{
			case TeamType::HERO:
			{
				switch (iteam.get_team())//Team of the other object
				{
					case TeamType::ENEMY:
					{
						rv = true;
						break;
					}

					default:
						break;
				}

				break;
			}

			case TeamType::ENEMY:
			{
				switch (iteam.get_team())//Team of the other object
				{
					case TeamType::HERO:
					{
						rv = true;
						break;
					}

					default:
						break;
				}
				break;
			}

			default:
				break;
		}

		return rv;
	}
	void HitCharAttack::hit(Hittable & h)
	{
		//If the enemy is another character
		HitCharacter *ptr_hitcharacter = dynamic_cast<HitCharacter *>(&h);
		if (ptr_hitcharacter)
		{
			//Check if they're enemies
			if (attack_team(*ptr_hitcharacter))
			{
				Ogre::Entity *ent = get_entity();
				
				if ((ent && anim != CharacterAnimState::DEATH && anim != CharacterAnimState::END) || !ent)
				{
					//Do damage
					if (deltaAttack == 0.0)
					{
						ptr_hitcharacter->takeDamage(dmg);
						deltaAttack = timeAttack;

						if (ent)
						{
							anim = CharacterAnimState::ATTACK;
							anim_controller = ent->getAnimationState("Attack2");
							anim_controller->setTimePosition(0);
							anim_controller->setLoop(false);
							anim_controller->setEnabled(true);
						}
					}
				}
			}
		}
	}

	bool HitCharAttack::finished()
	{
		if (HitCharacter::finished())
		{
			Ogre::Entity *ent = get_entity();

			if (ent)
			{
				switch (anim)
				{
					case CharacterAnimState::END:
						return true;
					case CharacterAnimState::DEATH:
						return false;
					default:
					{
						anim = CharacterAnimState::DEATH;
						anim_controller = ent->getAnimationState("Death1");
						anim_controller->setTimePosition(0);
						anim_controller->setLoop(false);
						anim_controller->setEnabled(true);
						return false;
					}
				}
			}
			else
				return true;
		}
		return false;
	}

	void HitCharAttack::deltaTime(double itime)
	{
		Ogre::Entity *ent = get_entity();
		if (ent)
		{
			switch(anim)
			{
				case ViRus::CharacterAnimState::START:
					anim = CharacterAnimState::WALK;
					anim_controller = ent->getAnimationState("Walk");
					anim_controller->setTimePosition(0);
					anim_controller->setLoop(true);
					anim_controller->setEnabled(true);
					break;
				case ViRus::CharacterAnimState::WALK:
					break;
				case ViRus::CharacterAnimState::ATTACK:
					if (anim_controller->hasEnded())
					{
						anim = CharacterAnimState::WALK;
						anim_controller = ent->getAnimationState("Walk");
						anim_controller->setTimePosition(0);
						anim_controller->setLoop(true);
						anim_controller->setEnabled(true);
					}
					break;
				case ViRus::CharacterAnimState::DEATH:
					if (anim_controller->hasEnded())
					{
						anim = CharacterAnimState::END;
					}
					break;
				case ViRus::CharacterAnimState::END:
					break;
				default:
					break;
			}

			if (anim_controller)
				anim_controller->addTime(itime);
		}


		deltaAttack -= itime;
		deltaAttack = std::max(0.0, deltaAttack);
	}
	void HitCharAttack::go_point(Ogre::Vector3 pos)
	{
		if (scene&&body)
		{
			Ogre::Vector3 dir = pos - scene->getPosition();
			dir.y = 0;
			dir.normalise();

			Ogre::Vector2 dir_2d(dir.x, dir.z);

			Ogre::Radian angle = dir_2d.angleTo(Ogre::Vector2::NEGATIVE_UNIT_Y);
			scene->resetOrientation();
			scene->yaw(angle);

			body->setLinearVelocity(dir*vel);
		}
	}
	void HitCharAttack::chase(const HitCharacter & h)
	{
		Ogre::Entity *ent = get_entity();
		if ((ent && anim != CharacterAnimState::DEATH && anim != CharacterAnimState::END) || !ent)
		{
			Ogre::Vector3 pos;
			if (h.get_position(pos))
				go_point(pos);
		}
		else
		{
			if (body)
				body->setLinearVelocity(Ogre::Vector3::ZERO);
		}
	}

	Ogre::Entity * HitCharAttack::get_entity()
	{
		return entity;
	}
	void HitCharAttack::set_entity(Ogre::Entity * ientity)
	{
		entity = ientity;
	}
	bool HitCharacter::get_position(Ogre::Vector3 & pos) const
	{
		if (!scene)
			return false;

		pos = scene->getPosition();
		return true;
	}

	void destroy_node(Ogre::SceneNode * node)
	{
		// Destroy all the attached objects
		Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

		while (itObject.hasMoreElements())
		{
			Ogre::MovableObject* pObject = static_cast<Ogre::MovableObject* > (itObject.getNext());
			node->getCreator()->destroyMovableObject(pObject);
		}

		//Recursive call on child nodes
		Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

		while (itChild.hasMoreElements())
		{
			Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
			destroy_node(pChildNode);
		}
	}
}