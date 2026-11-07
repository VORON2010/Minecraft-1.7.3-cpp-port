#include "world/entity/monster/Skeleton.h"
#include "world/stats/Achievement.h"
#include "world/stats/AchievementList.h"
#include "world/entity/player/Player.h"

#include <cmath>

#include "world/entity/projectile/EntityArrow.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "util/Mth.h"

namespace
{
	ItemInstance &skeletonHeldItem()
	{
		static ItemInstance held(Items::bow->getShiftedIndex(), 1, 0);
		return held;
	}
}

Skeleton::Skeleton(Level &level) : Monster(level)
{
	textureName = u"/mob/skeleton.png";
}

void Skeleton::aiStep()
{
	if (level.isDay())
	{
		float brightness = getBrightness(1.0f);
		if (brightness > 0.5f && level.canSeeSky(Mth::floor(x), Mth::floor(y), Mth::floor(z)) && random.nextFloat() * 30.0f < (brightness - 0.4f) * 2.0f)
			onFire = 300;
	}
	Monster::aiStep();
}

ItemInstance *Skeleton::getCarriedItem()
{
	return &skeletonHeldItem();
}

void Skeleton::checkHurtTarget(Entity &entity, float distance)
{
	if (distance >= 10.0f)
		return;
		double dx = entity.x - x;
	double dz = entity.z - z;
	if (attackTime == 0)
	{
		auto arrow = std::make_shared<EntityArrow>(level, *this);
		arrow->y += 1.4f;
		double dy = entity.y + entity.getHeadHeight() - 0.2f - arrow->y;
		float flat = Mth::sqrt(dx * dx + dz * dz) * 0.2f;
		level.playSoundAtEntity(*this, u"random.bow", 1.0f, 1.0f / (random.nextFloat() * 0.4f + 0.8f));
		level.addEntity(arrow);
		arrow->setArrowHeading(dx, dy + flat, dz, 0.6f, 12.0f);
		attackTime = 30;
	}
	yRot = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI) - 90.0f;
	holdGround = true;
}

void Skeleton::dropDeathLoot()
{
	Monster::dropDeathLoot();
	for (int_t i = 0, count = random.nextInt(3 + lootBonus); i < count; ++i)
		spawnAtLocation(ItemInstance(Items::bone->getShiftedIndex(), 1, 0), 0.0f);
}

jstring Skeleton::getAmbientSound()
{
	return u"mob.skeleton";
}

jstring Skeleton::getHurtSound()
{
	return u"mob.skeletonhurt";
}

jstring Skeleton::getDeathSound()
{
	return u"mob.skeletonhurt";
}

int_t Skeleton::getDeathLoot()
{
	return Items::arrow->getShiftedIndex();
}

void Skeleton::die(Entity *source)
{
	Monster::die(source);
	// 1.0 snipeSkeleton: an arrow from a player at least 50 blocks away
	Player *player = dynamic_cast<Player *>(source);
	if (player != nullptr && dynamic_cast<EntityArrow *>(Entity::damageProjectile) != nullptr)
	{
		double dx = player->x - x;
		double dz = player->z - z;
		if (dx * dx + dz * dz >= 2500.0)
			player->triggerAchievement(*AchievementList::snipeSkeleton);
	}
}
