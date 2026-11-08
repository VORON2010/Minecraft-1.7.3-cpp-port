#include "world/level/tile/ChestTile.h"

#include "client/player/LocalPlayer.h"
#include "util/Memory.h"
#include "util/Mth.h"
#include "world/CompoundContainer.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/entity/ChestTileEntity.h"

ChestTile::ChestTile(int_t id) : Tile(id, 26, Material::wood)
{
	Tile::isEntityTile[id] = true;
	updateDefaultShape();
	updateCachedProperties();
}

void ChestTile::updateDefaultShape()
{
	setShape(0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
}

void ChestTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	if (level.getTile(x, y, z - 1) == id)
		setShape(0.0625f, 0.0f, 0.0f, 0.9375f, 0.875f, 0.9375f);
	else if (level.getTile(x, y, z + 1) == id)
		setShape(0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 1.0f);
	else if (level.getTile(x - 1, y, z) == id)
		setShape(0.0f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
	else if (level.getTile(x + 1, y, z) == id)
		setShape(0.0625f, 0.0f, 0.0625f, 1.0f, 0.875f, 0.9375f);
	else
		setShape(0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
}

// 1.4.7 ChestBlock.onPlaced: face the player, and line up with a neighbouring chest
void ChestTile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t n = level.getTile(x, y, z - 1);
	int_t s = level.getTile(x, y, z + 1);
	int_t w = level.getTile(x - 1, y, z);
	int_t e = level.getTile(x + 1, y, z);
	int_t facing = 0;
	int_t dir = Mth::floor(player.yRot * 4.0f / 360.0f + 0.5) & 3;
	if (dir == 0) facing = 2;
	if (dir == 1) facing = 5;
	if (dir == 2) facing = 3;
	if (dir == 3) facing = 4;
	if (n != id && s != id && w != id && e != id)
	{
		level.setData(x, y, z, facing);
		return;
	}
	if ((n == id || s == id) && (facing == 4 || facing == 5))
	{
		if (n == id)
			level.setData(x, y, z - 1, facing);
		else
			level.setData(x, y, z + 1, facing);
		level.setData(x, y, z, facing);
	}
	if ((w == id || e == id) && (facing == 2 || facing == 3))
	{
		if (w == id)
			level.setData(x - 1, y, z, facing);
		else
			level.setData(x + 1, y, z, facing);
		level.setData(x, y, z, facing);
	}
}

// 1.4.7 ChestBlock.updateState: pick a facing away from solid blocks
void ChestTile::updateState(Level &level, int_t x, int_t y, int_t z)
{
	if (level.isOnline)
		return;
	int_t n = level.getTile(x, y, z - 1);
	int_t s = level.getTile(x, y, z + 1);
	int_t w = level.getTile(x - 1, y, z);
	int_t e = level.getTile(x + 1, y, z);
	int_t facing = 4;
	if (n == id || s == id)
	{
		int_t ww = level.getTile(x - 1, y, n == id ? z - 1 : z + 1);
		int_t ee = level.getTile(x + 1, y, n == id ? z - 1 : z + 1);
		facing = 5;
		int_t other = n == id ? level.getData(x, y, z - 1) : level.getData(x, y, z + 1);
		if (other == 4)
			facing = 4;
		if ((Tile::solid[w] || Tile::solid[ww]) && !Tile::solid[e] && !Tile::solid[ee])
			facing = 5;
		if ((Tile::solid[e] || Tile::solid[ee]) && !Tile::solid[w] && !Tile::solid[ww])
			facing = 4;
	}
	else if (w != id && e != id)
	{
		facing = 3;
		if (Tile::solid[n] && !Tile::solid[s])
			facing = 3;
		if (Tile::solid[s] && !Tile::solid[n])
			facing = 2;
		if (Tile::solid[w] && !Tile::solid[e])
			facing = 5;
		if (Tile::solid[e] && !Tile::solid[w])
			facing = 4;
	}
	else
	{
		int_t nn = level.getTile(w == id ? x - 1 : x + 1, y, z - 1);
		int_t ss = level.getTile(w == id ? x - 1 : x + 1, y, z + 1);
		facing = 3;
		int_t other = w == id ? level.getData(x - 1, y, z) : level.getData(x + 1, y, z);
		if (other == 2)
			facing = 2;
		if ((Tile::solid[n] || Tile::solid[nn]) && !Tile::solid[s] && !Tile::solid[ss])
			facing = 3;
		if ((Tile::solid[s] || Tile::solid[ss]) && !Tile::solid[n] && !Tile::solid[nn])
			facing = 2;
	}
	level.setData(x, y, z, facing);
}

void ChestTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	Tile::neighborChanged(level, x, y, z, tile);
	if (auto chest = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(x, y, z)))
		chest->clearCache();
}

int_t ChestTile::getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	(void)level; (void)x; (void)y; (void)z; (void)face;
	return 4;
}

int_t ChestTile::legacyTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
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
	(void)face;
	return 4;
}

void ChestTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.setTileEntity(x, y, z, Util::make_shared<ChestTileEntity>());
	// 1.4.7 ChestBlock.onAdded
	updateState(level, x, y, z);
	if (level.getTile(x, y, z - 1) == id) updateState(level, x, y, z - 1);
	if (level.getTile(x, y, z + 1) == id) updateState(level, x, y, z + 1);
	if (level.getTile(x - 1, y, z) == id) updateState(level, x - 1, y, z);
	if (level.getTile(x + 1, y, z) == id) updateState(level, x + 1, y, z);
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
