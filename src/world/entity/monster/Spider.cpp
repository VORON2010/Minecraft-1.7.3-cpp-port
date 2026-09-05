#include "world/entity/monster/Spider.h"

#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "util/Mth.h"

Spider::Spider(Level &level) : Monster(level)
{
	textureName = u"/mob/spider.png";
	setSize(1.4f, 0.9f);
	runSpeed = 0.8f;
	makeStepSound = false;
}

double Spider::getRideHeight()
{
	return bbHeight * 0.75 - 0.5;
}

bool Spider::onLadder()
{
	return horizontalCollision;
}

std::shared_ptr<Entity> Spider::findAttackTarget()
{
	if (getBrightness(1.0f) < 0.5f)
		return std::static_pointer_cast<Entity>(level.getNearestPlayer(*this, 16.0));
	return nullptr;
}

jstring Spider::getAmbientSound()
{
	return u"mob.spider";
}

jstring Spider::getHurtSound()
{
	return u"mob.spider";
}

jstring Spider::getDeathSound()
{
	return u"mob.spiderdeath";
}

void Spider::checkHurtTarget(Entity &entity, float distance)
{
	if (getBrightness(1.0f) > 0.5f && random.nextInt(100) == 0)
	{
		attackTarget = nullptr;
		return;
	}
	if ((distance <= 2.0f || distance >= 6.0f || random.nextInt(10) != 0) || !onGround)
	{
		Monster::checkHurtTarget(entity, distance);
		return;
	}

	double dx = entity.x - x;
	double dz = entity.z - z;
	float distanceMag = Mth::sqrt(dx * dx + dz * dz);
	xd = dx / distanceMag * 0.5 * 0.8f + xd * 0.2f;
	zd = dz / distanceMag * 0.5 * 0.8f + zd * 0.2f;
	yd = 0.4f;
}

int_t Spider::getDeathLoot()
{
	return Items::silk->getShiftedIndex();
}
