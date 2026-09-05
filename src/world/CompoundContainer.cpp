#include "world/CompoundContainer.h"

#include "world/entity/player/Player.h"
#include "world/level/tile/entity/ChestTileEntity.h"

CompoundContainer::CompoundContainer(jstring name, std::shared_ptr<ChestTileEntity> first, std::shared_ptr<ChestTileEntity> second)
	: name(std::move(name)), first(std::move(first)), second(std::move(second))
{
}

int_t CompoundContainer::getContainerSize() const
{
	return first->getContainerSize() + second->getContainerSize();
}

ItemInstance &CompoundContainer::getItem(int_t slot)
{
	int_t firstSize = first->getContainerSize();
	if (slot < firstSize)
		return first->getItem(slot);
	return second->getItem(slot - firstSize);
}

const ItemInstance &CompoundContainer::getItem(int_t slot) const
{
	int_t firstSize = first->getContainerSize();
	if (slot < firstSize)
		return first->getItem(slot);
	return second->getItem(slot - firstSize);
}

void CompoundContainer::setItem(int_t slot, const ItemInstance &item)
{
	int_t firstSize = first->getContainerSize();
	if (slot < firstSize)
	{
		first->setItem(slot, item);
		return;
	}
	second->setItem(slot - firstSize, item);
}

ItemInstance CompoundContainer::removeItem(int_t slot, int_t count)
{
	int_t firstSize = first->getContainerSize();
	if (slot < firstSize)
		return first->removeItem(slot, count);
	return second->removeItem(slot - firstSize, count);
}

bool CompoundContainer::canUse(Player &player) const
{
	return first->canUse(player) && second->canUse(player);
}

void CompoundContainer::setChanged()
{
	first->setChanged();
	second->setChanged();
}

const jstring &CompoundContainer::getName() const
{
	return name;
}
