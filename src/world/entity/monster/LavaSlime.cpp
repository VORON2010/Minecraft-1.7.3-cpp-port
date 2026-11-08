#include "world/entity/monster/LavaSlime.h"

#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

LavaSlime::LavaSlime(Level &level) : Slime(level)
{
	textureName = u"/mob/lava.png";
	fireImmune = true;
	runSpeed = 0.2f;
}

bool LavaSlime::canSpawn()
{
	return level.difficulty > 0 && level.isUnobstructed(bb) && level.getCubes(*this, bb).empty() && !level.containsAnyLiquid(bb);
}

// 1.0 getArmorProtection(): size * 3 points out of 25, applied in applyDamage
void LavaSlime::actuallyHurt(int_t dmg)
{
	int_t armor = 25 - getSlimeSize() * 3;
	int_t scaled = dmg * armor + armorCarry;
	armorCarry = scaled % 25;
	Slime::actuallyHurt(scaled / 25);
}

float LavaSlime::getBrightness(float a)
{
	(void)a;
	return 1.0f;
}

jstring LavaSlime::getParticleName()
{
	return u"flame";
}

std::shared_ptr<Slime> LavaSlime::createChild()
{
	return std::make_shared<LavaSlime>(level);
}

int_t LavaSlime::getDeathLoot()
{
	return 0;
}

bool LavaSlime::isOnFire()
{
	return false;
}

int_t LavaSlime::getJumpDelay()
{
	return Slime::getJumpDelay() * 4;
}

void LavaSlime::decreaseSquish()
{
	targetSquish *= 0.9f;
}

void LavaSlime::jumpFromGround()
{
	yd = 0.42f + getSlimeSize() * 0.1f;
}

void LavaSlime::causeFallDamage(float distance)
{
	(void)distance;
}

bool LavaSlime::isDealsDamage()
{
	return true;
}

int_t LavaSlime::getAttackDamage()
{
	return Slime::getAttackDamage() + 2;
}

jstring LavaSlime::getHurtSound()
{
	return getSlimeSize() > 1 ? u"mob.slime.big" : u"mob.slime.small";
}

jstring LavaSlime::getDeathSound()
{
	return getSlimeSize() > 1 ? u"mob.slime.big" : u"mob.slime.small";
}

jstring LavaSlime::getSquishSound()
{
	return getSlimeSize() > 1 ? u"mob.magmacube.big" : u"mob.magmacube.small";
}

bool LavaSlime::isInLava()
{
	return false;
}

bool LavaSlime::doPlayLandSound()
{
	return true;
}
