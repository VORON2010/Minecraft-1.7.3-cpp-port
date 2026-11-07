#include "world/entity/monster/Blaze.h"

#include <cmath>

#include "util/Mth.h"
#include "world/entity/projectile/SmallFireball.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

Blaze::Blaze(Level &level) : Monster(level)
{
	textureName = u"/mob/fire.png";
	fireImmune = true;
	attackDamage = 6;
	xpReward = 10;
	health = 20;
	dataWatcher.addObject(16, static_cast<byte_t>(0));
}

jstring Blaze::getAmbientSound()
{
	return u"mob.blaze.breathe";
}

jstring Blaze::getHurtSound()
{
	return u"mob.blaze.hit";
}

jstring Blaze::getDeathSound()
{
	return u"mob.blaze.death";
}

float Blaze::getBrightness(float a)
{
	(void)a;
	return 1.0f;
}

void Blaze::aiStep()
{
	if (!level.isOnline)
	{
		if (isWet())
			hurt(nullptr, 1);
		nextHeightOffsetChangeTick--;
		if (nextHeightOffsetChangeTick <= 0)
		{
			nextHeightOffsetChangeTick = 100;
			allowedHeightOffset = 0.5f + static_cast<float>(random.nextGaussian()) * 3.0f;
		}
		if (attackTarget != nullptr && attackTarget->y + attackTarget->getHeadHeight() > y + getHeadHeight() + allowedHeightOffset)
			yd = yd + (0.3f - yd) * 0.3f;
	}

	if (random.nextInt(24) == 0)
		level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, u"fire.fire", 1.0f + random.nextFloat(), random.nextFloat() * 0.7f + 0.3f);

	if (!onGround && yd < 0.0)
		yd *= 0.6;

	for (int_t i = 0; i < 2; i++)
		level.addParticle(u"largesmoke", x + (random.nextDouble() - 0.5) * bbWidth, y + random.nextDouble() * bbHeight, z + (random.nextDouble() - 0.5) * bbWidth, 0.0, 0.0, 0.0);

	Monster::aiStep();
}

void Blaze::checkHurtTarget(Entity &target, float distance)
{
	if (attackTime <= 0 && distance < 2.0f && target.bb.y1 > bb.y0 && target.bb.y0 < bb.y1)
	{
		attackTime = 20;
		target.hurt(this, attackDamage);
	}
	else if (distance < 30.0f)
	{
		double dx = target.x - x;
		double dy = target.bb.y0 + target.bbHeight / 2.0f - (y + bbHeight / 2.0f);
		double dz = target.z - z;
		if (attackTime == 0)
		{
			attackCounter++;
			if (attackCounter == 1)
			{
				attackTime = 60;
				setCharged(true);
			}
			else if (attackCounter <= 4)
			{
				attackTime = 6;
			}
			else
			{
				attackTime = 100;
				attackCounter = 0;
				setCharged(false);
			}

			if (attackCounter > 1)
			{
				float spread = Mth::sqrt(distance) * 0.5f;
				level.levelEvent(1009, static_cast<int_t>(x), static_cast<int_t>(y), static_cast<int_t>(z), 0);
				for (int_t i = 0; i < 1; i++)
				{
					auto fireball = std::make_shared<SmallFireball>(level, *this, dx + random.nextGaussian() * spread, dy, dz + random.nextGaussian() * spread);
					fireball->y = y + bbHeight / 2.0f + 0.5;
					level.addEntity(fireball);
				}
			}
		}
		yRot = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI) - 90.0f;
		holdGround = true;
	}
}

void Blaze::causeFallDamage(float distance)
{
	(void)distance;
}

int_t Blaze::getDeathLoot()
{
	return Items::blazeRod->getShiftedIndex();
}

void Blaze::dropDeathLoot()
{
	if (lastHurtByPlayerTime <= 0)
		return;
	int_t count = random.nextInt(2);
	for (int_t i = 0; i < count; i++)
		spawnAtLocation(ItemInstance(Items::blazeRod->getShiftedIndex(), 1, 0), 0.0f);
}

bool Blaze::isOnFire()
{
	return isCharged();
}

bool Blaze::isCharged()
{
	return (dataWatcher.getWatchableObjectByte(16) & 1) != 0;
}

void Blaze::setCharged(bool charged)
{
	byte_t flags = dataWatcher.getWatchableObjectByte(16);
	flags = charged ? static_cast<byte_t>(flags | 1) : static_cast<byte_t>(flags & ~1);
	dataWatcher.updateObject(16, flags);
}

bool Blaze::canSpawn()
{
	return PathfinderMob::canSpawn();
}
