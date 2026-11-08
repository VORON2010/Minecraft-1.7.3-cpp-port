#include "world/entity/AgableMob.h"

#include "nbt/CompoundTag.h"
#include "world/entity/EntityIO.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/Level.h"

bool AgableMob::interact(Player &player)
{
	ItemInstance *held = player.inventory.getSelected();
	if (held != nullptr && held->itemID == Items::spawnEgg->getShiftedIndex() && !level.isOnline)
	{
		std::shared_ptr<Entity> sample = EntityIO::newEntity(held->getAuxValue(), level);
		if (sample != nullptr && typeid(*sample) == typeid(*this))
		{
			std::shared_ptr<AgableMob> child = getBreedOffspring(*this);
			if (child != nullptr)
			{
				child->setAge(-24000);
				child->moveTo(x, y, z, 0.0f, 0.0f);
				level.addEntity(child);
				if (!player.abilities.instabuild)
				{
					held->stackSize--;
					if (held->stackSize <= 0)
						player.inventory.setItem(player.inventory.currentItem, ItemInstance());
				}
			}
		}
	}
	return PathfinderMob::interact(player);
}

void AgableMob::setAge(int_t value)
{
	age = value;
	setBaby(isBaby());
}

void AgableMob::aiStep()
{
	PathfinderMob::aiStep();
	if (level.isOnline)
		setBaby(isBaby());
	else if (age < 0)
		setAge(age + 1);
	else if (age > 0)
		setAge(age - 1);
}

void AgableMob::setSize(float width, float height)
{
	bool registered = registeredBBWidth > 0.0f;
	registeredBBWidth = width;
	registeredBBHeight = height;
	if (!registered)
		updateSize(1.0f);
}

void AgableMob::updateSize(float scale)
{
	PathfinderMob::setSize(registeredBBWidth * scale, registeredBBHeight * scale);
}

void AgableMob::addAdditionalSaveData(CompoundTag &tag)
{
	PathfinderMob::addAdditionalSaveData(tag);
	tag.putInt(u"Age", age);
}

void AgableMob::readAdditionalSaveData(CompoundTag &tag)
{
	PathfinderMob::readAdditionalSaveData(tag);
	setAge(tag.getInt(u"Age"));
}
