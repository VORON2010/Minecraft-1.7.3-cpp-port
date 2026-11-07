#include "world/entity/npc/Villager.h"

#include "nbt/CompoundTag.h"

Villager::Villager(Level &level) : Villager(level, 0)
{
}

Villager::Villager(Level &level, int_t profession) : PathfinderMob(level), profession(profession)
{
	updateTexture();
	runSpeed = 0.5f;
	health = 20;
}

void Villager::addAdditionalSaveData(CompoundTag &tag)
{
	PathfinderMob::addAdditionalSaveData(tag);
	tag.putInt(u"Profession", profession);
}

void Villager::readAdditionalSaveData(CompoundTag &tag)
{
	PathfinderMob::readAdditionalSaveData(tag);
	profession = tag.getInt(u"Profession");
	updateTexture();
}

void Villager::updateTexture()
{
	textureName = u"/mob/villager/villager.png";
	if (profession == 0)
		textureName = u"/mob/villager/farmer.png";
	if (profession == 1)
		textureName = u"/mob/villager/librarian.png";
	if (profession == 2)
		textureName = u"/mob/villager/priest.png";
	if (profession == 3)
		textureName = u"/mob/villager/smith.png";
	if (profession == 4)
		textureName = u"/mob/villager/butcher.png";
}

bool Villager::canDespawn()
{
	return false;
}

jstring Villager::getAmbientSound()
{
	return u"mob.villager.default";
}

jstring Villager::getHurtSound()
{
	return u"mob.villager.defaulthurt";
}

jstring Villager::getDeathSound()
{
	return u"mob.villager.defaultdeath";
}
