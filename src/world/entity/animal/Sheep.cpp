#include "world/entity/animal/Sheep.h"

#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/level/tile/Tile.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "world/level/tile/ClothTile.h"

Sheep::Sheep(Level &level) : Animal(level)
{
	dataWatcher.addObject(16, static_cast<byte_t>(0));
	textureName = u"/mob/sheep.png";
	setSize(0.9f, 1.3f);
}

bool Sheep::interact(Player &player)
{
	ItemInstance *selected = player.getSelectedItem();
	if (selected != nullptr && selected->itemID == Items::shears->getShiftedIndex() && !isSheared())
	{
		if (!level.isOnline)
		{
			setSheared(true);
			int_t count = 2 + random.nextInt(3);
			for (int_t i = 0; i < count; ++i)
			{
				auto dropped = std::make_shared<EntityItem>(level, x, y + 1.0, z, ItemInstance(Tile::wool.id, 1, getFleeceColor()));
				dropped->yd += random.nextFloat() * 0.05f;
				dropped->xd += (random.nextFloat() - random.nextFloat()) * 0.1f;
				dropped->zd += (random.nextFloat() - random.nextFloat()) * 0.1f;
				level.addEntity(dropped);
			}
		}
		selected->damageItem(1, player);
		if (selected->isEmpty())
			player.removeSelectedItem();
	}
	return false;
}

void Sheep::dropDeathLoot()
{
	if (!isSheared())
		spawnAtLocation(ItemInstance(Tile::wool.id, 1, getFleeceColor()), 0.0f);
}

void Sheep::addAdditionalSaveData(CompoundTag &tag)
{
	Animal::addAdditionalSaveData(tag);
	tag.putBoolean(u"Sheared", isSheared());
	tag.putByte(u"Color", static_cast<byte_t>(getFleeceColor()));
}

void Sheep::readAdditionalSaveData(CompoundTag &tag)
{
	Animal::readAdditionalSaveData(tag);
	setSheared(tag.getBoolean(u"Sheared"));
	setFleeceColor(tag.getByte(u"Color"));
}

jstring Sheep::getAmbientSound()
{
	return u"mob.sheep";
}

jstring Sheep::getHurtSound()
{
	return u"mob.sheep";
}

jstring Sheep::getDeathSound()
{
	return u"mob.sheep";
}

int_t Sheep::getFleeceColor() const
{
	return dataWatcher.getWatchableObjectByte(16) & 15;
}

void Sheep::setFleeceColor(int_t color)
{
	byte_t value = dataWatcher.getWatchableObjectByte(16);
	dataWatcher.updateObject(16, static_cast<byte_t>((value & 240) | (color & 15)));
}

bool Sheep::isSheared() const
{
	return (dataWatcher.getWatchableObjectByte(16) & 16) != 0;
}

void Sheep::setSheared(bool sheared)
{
	byte_t value = dataWatcher.getWatchableObjectByte(16);
	if (sheared)
		dataWatcher.updateObject(16, static_cast<byte_t>(value | 16));
	else
		dataWatcher.updateObject(16, static_cast<byte_t>(value & -17));
}

int_t Sheep::getRandomFleeceColor(Random &random)
{
	int_t value = random.nextInt(100);
	if (value < 5) return 15;
	if (value < 10) return 7;
	if (value < 15) return 8;
	if (value < 18) return 12;
	return random.nextInt(500) == 0 ? 6 : 0;
}
