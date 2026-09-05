#include "world/entity/monster/PigZombie.h"

#include "world/item/Item.h"
#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

namespace
{
	ItemInstance &pigZombieHeldItem()
	{
		static ItemInstance held(Items::swordGold->getShiftedIndex(), 1, 0);
		return held;
	}
}

PigZombie::PigZombie(Level &level) : Zombie(level)
{
	textureName = u"/mob/pigzombie.png";
	runSpeed = 0.5f;
	attackDamage = 5;
	fireImmune = true;
}

void PigZombie::tick()
{
	runSpeed = attackTarget != nullptr ? 0.95f : 0.5f;
	if (playAngrySoundIn > 0 && --playAngrySoundIn == 0)
		level.playSoundAtEntity(*this, u"mob.zombiepig.zpigangry", getSoundVolume() * 2.0f, ((random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f) * 1.8f);
	Zombie::tick();
}

bool PigZombie::hurt(Entity *source, int_t damage)
{
	if (dynamic_cast<Player *>(source) != nullptr)
	{
		AABB *angerBox = bb.grow(32.0, 32.0, 32.0);
		const auto &nearby = level.getEntities(this, *angerBox);
		delete angerBox;
		for (const auto &entity : nearby)
		{
			PigZombie *pigZombie = dynamic_cast<PigZombie *>(entity.get());
			if (pigZombie != nullptr)
				pigZombie->becomeAngryAt(*source);
		}
		becomeAngryAt(*source);
	}
	return Zombie::hurt(source, damage);
}

bool PigZombie::canSpawn()
{
	return level.difficulty > 0 && level.isUnobstructed(bb) && level.getCubes(*this, bb).empty() && !level.containsAnyLiquid(bb);
}

ItemInstance *PigZombie::getCarriedItem()
{
	return &pigZombieHeldItem();
}

void PigZombie::addAdditionalSaveData(CompoundTag &tag)
{
	Zombie::addAdditionalSaveData(tag);
	tag.putShort(u"Anger", static_cast<short_t>(angerTime));
}

void PigZombie::readAdditionalSaveData(CompoundTag &tag)
{
	Zombie::readAdditionalSaveData(tag);
	angerTime = tag.getShort(u"Anger");
}

std::shared_ptr<Entity> PigZombie::findAttackTarget()
{
	if (angerTime == 0)
		return nullptr;
	return Zombie::findAttackTarget();
}

jstring PigZombie::getAmbientSound()
{
	return u"mob.zombiepig.zpig";
}

jstring PigZombie::getHurtSound()
{
	return u"mob.zombiepig.zpighurt";
}

jstring PigZombie::getDeathSound()
{
	return u"mob.zombiepig.zpigdeath";
}

int_t PigZombie::getDeathLoot()
{
	return Items::porkchopCooked->getShiftedIndex();
}

void PigZombie::becomeAngryAt(Entity &target)
{
	attackTarget = level.getEntityRef(target);
	angerTime = 400 + random.nextInt(400);
	playAngrySoundIn = random.nextInt(40);
}
