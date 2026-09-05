#include "world/entity/monster/Monster.h"

#include "world/level/Level.h"
#include "world/level/LightLayer.h"
#include "util/Mth.h"

Monster::Monster(Level &level) : PathfinderMob(level)
{
	setSize(0.6f, 1.8f);
	health = 20;
}

void Monster::tick()
{
	Mob::tick();
	if (!level.isOnline && level.difficulty == 0)
		remove();
}

void Monster::aiStep()
{
	if (getBrightness(1.0f) > 0.5f)
		noActionTime += 2;
	PathfinderMob::aiStep();
}

float Monster::getHeadHeight()
{
	return bbHeight * 0.85f;
}

bool Monster::hurt(Entity *source, int_t damage)
{
	if (!Mob::hurt(source, damage))
		return false;
	if (source != nullptr && rider.get() != source && riding.get() != source && source != this)
		attackTarget = level.getEntityRef(*source);
	return true;
}

std::shared_ptr<Entity> Monster::findAttackTarget()
{
	std::shared_ptr<Player> player = level.getNearestPlayer(*this, 16.0);
	if (player != nullptr && canSee(*player))
		return std::static_pointer_cast<Entity>(player);
	return nullptr;
}

void Monster::checkHurtTarget(Entity &entity, float distance)
{
	if (attackTime <= 0 && distance < 2.0f && entity.bb.y1 > bb.y0 && entity.bb.y0 < bb.y1)
	{
		attackTime = 20;
		entity.hurt(this, attackDamage);
	}
}

float Monster::getWalkTargetValue(int_t x, int_t y, int_t z)
{
	return 0.5f - level.getBrightness(x, y, z);
}

bool Monster::canSpawn()
{
	int_t x = Mth::floor(this->x);
	int_t y = Mth::floor(bb.y0);
	int_t z = Mth::floor(this->z);
	if (level.getBrightness(LightLayer::Sky, x, y, z) > random.nextInt(32))
		return false;
	int_t light = level.getRawBrightness(x, y, z);
	if (level.isThundering())
	{
		int_t oldDarken = level.skyDarken;
		level.skyDarken = 10;
		light = level.getRawBrightness(x, y, z);
		level.skyDarken = oldDarken;
	}
	return light <= random.nextInt(8) && PathfinderMob::canSpawn();
}
