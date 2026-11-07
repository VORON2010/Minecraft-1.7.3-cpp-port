#include "world/entity/monster/CaveSpider.h"

#include "world/level/Level.h"

CaveSpider::CaveSpider(Level &level) : Spider(level)
{
	textureName = u"/mob/cavespider.png";
	setSize(0.7f, 0.5f);
	health = 12;
}

void CaveSpider::checkHurtTarget(Entity &entity, float distance)
{
	int_t before = attackTime;
	Spider::checkHurtTarget(entity, distance);
	bool hit = before <= 0 && attackTime > 0;
	if (!hit)
		return;

	Mob *target = dynamic_cast<Mob *>(&entity);
	if (target == nullptr)
		return;
	int_t seconds = 0;
	if (level.difficulty == 2)
		seconds = 7;
	else if (level.difficulty == 3)
		seconds = 15;
	if (seconds > 0)
		target->addEffect(MobEffectInstance(MobEffect::poison.id, seconds * 20, 0));
}
