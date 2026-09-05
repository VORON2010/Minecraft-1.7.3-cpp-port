#include "world/level/tile/ChestTile.h"

#include "client/player/LocalPlayer.h"
#include "util/Memory.h"
#include "world/CompoundContainer.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/entity/ChestTileEntity.h"

ChestTile::ChestTile(int_t id) : Tile(id, 26, Material::wood)
{
	Tile::isEntityTile[id] = true;
	updateCachedProperties();
}

int_t ChestTile::getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	if (face == Facing::UP || face == Facing::DOWN)
		return tex - 1;

	int_t north = level.getTile(x, y, z - 1);
	int_t south = level.getTile(x, y, z + 1);
	int_t west = level.getTile(x - 1, y, z);
	int_t east = level.getTile(x + 1, y, z);

	if (north != id && south != id)
	{
		if (west != id && east != id)
		{
			Facing front = Facing::SOUTH;
			if (Tile::solid[north] && !Tile::solid[south])
				front = Facing::SOUTH;
			if (Tile::solid[south] && !Tile::solid[north])
				front = Facing::NORTH;
			if (Tile::solid[west] && !Tile::solid[east])
				front = Facing::EAST;
			if (Tile::solid[east] && !Tile::solid[west])
				front = Facing::WEST;
			return face == front ? tex + 1 : tex;
		}
		if (face == Facing::WEST || face == Facing::EAST)
			return tex;

		int_t offset = 0;
		if (west == id)
			offset = -1;

		int_t northSide = level.getTile(west == id ? x - 1 : x + 1, y, z - 1);
		int_t southSide = level.getTile(west == id ? x - 1 : x + 1, y, z + 1);
		if (face == Facing::SOUTH)
			offset = -1 - offset;

		Facing front = Facing::SOUTH;
		if ((Tile::solid[north] || Tile::solid[northSide]) && !Tile::solid[south] && !Tile::solid[southSide])
			front = Facing::SOUTH;
		if ((Tile::solid[south] || Tile::solid[southSide]) && !Tile::solid[north] && !Tile::solid[northSide])
			front = Facing::NORTH;
		return (face == front ? tex + 16 : tex + 32) + offset;
	}

	if (face == Facing::NORTH || face == Facing::SOUTH)
		return tex;

	int_t offset = 0;
	if (north == id)
		offset = -1;

	int_t westSide = level.getTile(x - 1, y, north == id ? z - 1 : z + 1);
	int_t eastSide = level.getTile(x + 1, y, north == id ? z - 1 : z + 1);
	if (face == Facing::WEST)
		offset = -1 - offset;

	Facing front = Facing::WEST;
	if ((Tile::solid[west] || Tile::solid[westSide]) && !Tile::solid[east] && !Tile::solid[eastSide])
		front = Facing::WEST;
	if ((Tile::solid[east] || Tile::solid[eastSide]) && !Tile::solid[west] && !Tile::solid[westSide])
		front = Facing::EAST;
	return (face == front ? tex + 16 : tex + 32) + offset;
}

int_t ChestTile::getTexture(Facing face)
{
	if (face == Facing::UP || face == Facing::DOWN)
		return tex - 1;
	return face == Facing::SOUTH ? tex + 1 : tex;
}

void ChestTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.setTileEntity(x, y, z, Util::make_shared<ChestTileEntity>());
}

void ChestTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	dropContents(level, x, y, z);
	level.removeTileEntity(x, y, z);
}

bool ChestTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (isBlockedChest(level, x, y, z)
		|| (level.getTile(x - 1, y, z) == id && isBlockedChest(level, x - 1, y, z))
		|| (level.getTile(x + 1, y, z) == id && isBlockedChest(level, x + 1, y, z))
		|| (level.getTile(x, y, z - 1) == id && isBlockedChest(level, x, y, z - 1))
		|| (level.getTile(x, y, z + 1) == id && isBlockedChest(level, x, y, z + 1)))
	{
		return true;
	}

	
	
	
	if (level.isOnline)
		return true;

	auto chest = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(x, y, z));
	if (chest == nullptr)
		return false;

	LocalPlayer *localPlayer = dynamic_cast<LocalPlayer *>(&player);
	if (localPlayer == nullptr)
		return false;

	if (level.getTile(x - 1, y, z) == id)
	{
		auto other = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(x - 1, y, z));
		if (other != nullptr)
		{
			localPlayer->startChest(std::make_shared<CompoundContainer>(u"Large chest", other, chest));
			return true;
		}
	}
	if (level.getTile(x + 1, y, z) == id)
	{
		auto other = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(x + 1, y, z));
		if (other != nullptr)
		{
			localPlayer->startChest(std::make_shared<CompoundContainer>(u"Large chest", chest, other));
			return true;
		}
	}
	if (level.getTile(x, y, z - 1) == id)
	{
		auto other = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(x, y, z - 1));
		if (other != nullptr)
		{
			localPlayer->startChest(std::make_shared<CompoundContainer>(u"Large chest", other, chest));
			return true;
		}
	}
	if (level.getTile(x, y, z + 1) == id)
	{
		auto other = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(x, y, z + 1));
		if (other != nullptr)
		{
			localPlayer->startChest(std::make_shared<CompoundContainer>(u"Large chest", chest, other));
			return true;
		}
	}

	localPlayer->startChest(chest);
	return true;
}

bool ChestTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	int_t adjacentChests = 0;
	if (level.getTile(x - 1, y, z) == id)
		adjacentChests++;
	if (level.getTile(x + 1, y, z) == id)
		adjacentChests++;
	if (level.getTile(x, y, z - 1) == id)
		adjacentChests++;
	if (level.getTile(x, y, z + 1) == id)
		adjacentChests++;
	if (adjacentChests > 1)
		return false;

	return !hasNeighborChest(level, x - 1, y, z)
		&& !hasNeighborChest(level, x + 1, y, z)
		&& !hasNeighborChest(level, x, y, z - 1)
		&& !hasNeighborChest(level, x, y, z + 1);
}

bool ChestTile::hasNeighborChest(Level &level, int_t x, int_t y, int_t z) const
{
	if (level.getTile(x, y, z) != id)
		return false;
	return level.getTile(x - 1, y, z) == id
		|| level.getTile(x + 1, y, z) == id
		|| level.getTile(x, y, z - 1) == id
		|| level.getTile(x, y, z + 1) == id;
}

bool ChestTile::isBlockedChest(Level &level, int_t x, int_t y, int_t z) const
{
	return level.isBlockNormalCube(x, y + 1, z);
}

void ChestTile::dropContents(Level &level, int_t x, int_t y, int_t z) const
{
	auto chest = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(x, y, z));
	if (chest == nullptr)
		return;

	for (int_t slot = 0; slot < chest->getContainerSize(); ++slot)
	{
		ItemInstance &stack = chest->getItem(slot);
		if (stack.isEmpty())
			continue;

		float xo = level.random.nextFloat() * 0.8f + 0.1f;
		float yo = level.random.nextFloat() * 0.8f + 0.1f;
		float zo = level.random.nextFloat() * 0.8f + 0.1f;
		while (!stack.isEmpty())
		{
			int_t amount = level.random.nextInt(21) + 10;
			if (amount > stack.stackSize)
				amount = stack.stackSize;
			ItemInstance dropped = stack.remove(amount);
			auto entity = std::make_shared<EntityItem>(level, static_cast<double>(x) + xo, static_cast<double>(y) + yo, static_cast<double>(z) + zo, dropped);
			float spread = 0.05f;
			entity->xd = (level.random.nextFloat() * 2.0f - 1.0f) * spread;
			entity->yd = (level.random.nextFloat() * 2.0f - 1.0f) * spread + 0.2f;
			entity->zd = (level.random.nextFloat() * 2.0f - 1.0f) * spread;
			level.addEntity(entity);
		}
		stack = ItemInstance();
}
}
