#include "world/entity/animal/Pig.h"

#include "world/entity/monster/PigZombie.h"
#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "world/stats/AchievementList.h"
#include "world/stats/Achievement.h"

Pig::Pig(Level &level) : Animal(level)
{
	dataWatcher.addObject(16, static_cast<byte_t>(0));
	textureName = u"/mob/pig.png";
	setSize(0.9f, 0.9f);
}

bool Pig::interact(Player &player)
{
	if (!isSaddled() || level.isOnline || (rider != nullptr && rider.get() != &player))
		return false;
	auto self = level.getEntityRef(*this);
	if (self == nullptr)
		return false;
	player.ride(self);
	return true;
}

void Pig::onStruckByLightning(Entity &lightning)
{
	if (level.isOnline)
		return;
	auto pigZombie = std::make_shared<PigZombie>(level);
	pigZombie->moveTo(x, y, z, yRot, xRot);
	level.addEntity(pigZombie);
	remove();
}

void Pig::causeFallDamage(float distance)
{
	Animal::causeFallDamage(distance);
	if (distance > 5.0f && rider != nullptr)
	{
		if (Player *player = dynamic_cast<Player *>(rider.get()))
			player->triggerAchievement(*AchievementList::flyPig);
	}
}

void Pig::addAdditionalSaveData(CompoundTag &tag)
{
	Animal::addAdditionalSaveData(tag);
	tag.putBoolean(u"Saddle", isSaddled());
}

void Pig::readAdditionalSaveData(CompoundTag &tag)
{
	Animal::readAdditionalSaveData(tag);
	setSaddled(tag.getBoolean(u"Saddle"));
}

jstring Pig::getAmbientSound()
{
	return u"mob.pig";
}

jstring Pig::getHurtSound()
{
	return u"mob.pig";
}

jstring Pig::getDeathSound()
{
	return u"mob.pigdeath";
}

int_t Pig::getDeathLoot()
{
	return onFire > 0 ? Items::porkchopCooked->getShiftedIndex() : Items::porkchopRaw->getShiftedIndex();
}

bool Pig::isSaddled() const
{
	return (dataWatcher.getWatchableObjectByte(16) & 1) != 0;
}

void Pig::setSaddled(bool saddled)
{
	dataWatcher.updateObject(16, static_cast<byte_t>(saddled ? 1 : 0));
}
