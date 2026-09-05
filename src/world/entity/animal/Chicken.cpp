#include "world/entity/animal/Chicken.h"

#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

Chicken::Chicken(Level &level) : Animal(level)
{
	textureName = u"/mob/chicken.png";
	setSize(0.3f, 0.4f);
	health = 4;
	eggTime = random.nextInt(6000) + 6000;
}

void Chicken::aiStep()
{
	Animal::aiStep();
	oFlapSpeed = flap;
	oFlap = flapSpeed;
	flapSpeed += (onGround ? -1.0f : 4.0f) * 0.3f;
	if (flapSpeed < 0.0f) flapSpeed = 0.0f;
	if (flapSpeed > 1.0f) flapSpeed = 1.0f;
	if (!onGround && flapping < 1.0f)
		flapping = 1.0f;
	flapping *= 0.9f;
	if (!onGround && yd < 0.0)
		yd *= 0.6;
	flap += flapping * 2.0f;
	if (!level.isOnline && --eggTime <= 0)
	{
		level.playSoundAtEntity(*this, u"mob.chickenplop", 1.0f, (random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
		spawnAtLocation(ItemInstance(Items::egg->getShiftedIndex(), 1, 0), 0.0f);
		eggTime = random.nextInt(6000) + 6000;
	}
}

void Chicken::causeFallDamage(float distance)
{
	(void)distance;
}

jstring Chicken::getAmbientSound()
{
	return u"mob.chicken";
}

jstring Chicken::getHurtSound()
{
	return u"mob.chickenhurt";
}

jstring Chicken::getDeathSound()
{
	return u"mob.chickenhurt";
}

int_t Chicken::getDeathLoot()
{
	return Items::feather->getShiftedIndex();
}
