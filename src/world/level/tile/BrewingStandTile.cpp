#include "world/level/tile/BrewingStandTile.h"
#include "world/item/Item.h"

#include "client/player/LocalPlayer.h"
#include "util/Memory.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/entity/BrewingStandTileEntity.h"

BrewingStandTile::BrewingStandTile(int_t id) : Tile(id, 157, Material::iron)
{
	// non-solid: lets light through and keeps neighbour faces
	updateCachedProperties();
	Tile::isEntityTile[id] = true;
}

void BrewingStandTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	setShape(0.4375f, 0.0f, 0.4375f, 0.5625f, 0.875f, 0.5625f);
	Tile::addAABBs(level, x, y, z, bb, aabbList);
	updateDefaultShape();
	Tile::addAABBs(level, x, y, z, bb, aabbList);
}

void BrewingStandTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
}

void BrewingStandTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onPlace(level, x, y, z);
	level.setTileEntity(x, y, z, Util::make_shared<BrewingStandTileEntity>());
}

void BrewingStandTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	auto stand = std::dynamic_pointer_cast<BrewingStandTileEntity>(level.getTileEntity(x, y, z));
	if (stand != nullptr)
	{
		for (int_t slot = 0; slot < stand->getSizeInventory(); ++slot)
		{
			ItemInstance &stack = stand->getItem(slot);
			if (stack.isEmpty())
				continue;
			float xo = random.nextFloat() * 0.8f + 0.1f;
			float yo = random.nextFloat() * 0.8f + 0.1f;
			float zo = random.nextFloat() * 0.8f + 0.1f;
			while (stack.stackSize > 0)
			{
				int_t amount = random.nextInt(21) + 10;
				if (amount > stack.stackSize)
					amount = stack.stackSize;
				stack.stackSize -= amount;
				ItemInstance dropped(stack.itemID, amount, stack.itemDamage);
				auto entity = std::make_shared<EntityItem>(level, x + xo, y + yo, z + zo, dropped);
				float velocity = 0.05f;
				entity->xd = static_cast<float>(random.nextGaussian()) * velocity;
				entity->yd = static_cast<float>(random.nextGaussian()) * velocity + 0.2f;
				entity->zd = static_cast<float>(random.nextGaussian()) * velocity;
				level.addEntity(entity);
			}
			stack = ItemInstance();
		}
	}
	level.removeTileEntity(x, y, z);
}

bool BrewingStandTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (level.isOnline)
		return true;
	auto stand = std::dynamic_pointer_cast<BrewingStandTileEntity>(level.getTileEntity(x, y, z));
	LocalPlayer *localPlayer = dynamic_cast<LocalPlayer *>(&player);
	if (stand != nullptr && localPlayer != nullptr)
		localPlayer->startBrewing(stand);
	return true;
}

void BrewingStandTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	double px = x + 0.4f + random.nextFloat() * 0.2f;
	double py = y + 0.7f + random.nextFloat() * 0.3f;
	double pz = z + 0.4f + random.nextFloat() * 0.2f;
	level.addParticle(u"smoke", px, py, pz, 0.0, 0.0, 0.0);
}

int_t BrewingStandTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Items::brewingStand->getShiftedIndex();
}

CauldronTile::CauldronTile(int_t id) : Tile(id, 154, Material::iron)
{
	// non-solid: lets light through and keeps neighbour faces
	updateCachedProperties();
}

int_t CauldronTile::getTexture(Facing face, int_t data)
{
	(void)data;
	if (face == Facing::UP)
		return 138;
	return face == Facing::DOWN ? 155 : 154;
}

void CauldronTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.3125f, 1.0f);
	Tile::addAABBs(level, x, y, z, bb, aabbList);
	float w = 0.125f;
	setShape(0.0f, 0.0f, 0.0f, w, 1.0f, 1.0f);
	Tile::addAABBs(level, x, y, z, bb, aabbList);
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, w);
	Tile::addAABBs(level, x, y, z, bb, aabbList);
	setShape(1.0f - w, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	Tile::addAABBs(level, x, y, z, bb, aabbList);
	setShape(0.0f, 0.0f, 1.0f - w, 1.0f, 1.0f, 1.0f);
	Tile::addAABBs(level, x, y, z, bb, aabbList);
	updateDefaultShape();
}

void CauldronTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

bool CauldronTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (level.isOnline)
		return true;
	ItemInstance *held = player.inventory.getCurrentItem();
	if (held == nullptr)
		return true;
	int_t data = level.getData(x, y, z);
	if (held->itemID == Items::bucketWater->getShiftedIndex())
	{
		if (data < 3)
		{
			if (!player.abilities.instabuild)
				player.inventory.setItem(player.inventory.currentItem, ItemInstance(Items::bucketEmpty->getShiftedIndex(), 1, 0));
			level.setData(x, y, z, 3);
		}
		return true;
	}
	if (held->itemID == Items::glassBottle->getShiftedIndex() && data > 0)
	{
		ItemInstance water(Items::potion->getShiftedIndex(), 1, 0);
		if (!player.inventory.add(water))
			level.addEntity(std::make_shared<EntityItem>(level, x + 0.5, y + 1.5, z + 0.5, water));
		held->stackSize--;
		if (held->stackSize <= 0)
			player.inventory.setItem(player.inventory.currentItem, ItemInstance());
		level.setData(x, y, z, data - 1);
	}
	return true;
}

int_t CauldronTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Items::cauldron->getShiftedIndex();
}
