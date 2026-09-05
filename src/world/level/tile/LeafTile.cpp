#include "world/level/tile/LeafTile.h"

#include "world/level/Level.h"

#include "world/level/FoliageColor.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/SaplingTile.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/entity/player/Player.h"
#include "world/entity/item/EntityItem.h"
#include "world/stats/StatList.h"

LeafTile::LeafTile(int_t id, int_t tex) : TransparentTile(id, tex, Material::leaves, false)
{
	oTex = tex;
	setTicking(true);
	updateCachedProperties();
}

int_t LeafTile::getColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t type = level.getData(x, y, z) & LEAF_TYPE_MASK;
	if (type == SPRUCE_LEAF)
		return FoliageColor::getEvergreenColor();
	if (type == BIRCH_LEAF)
		return FoliageColor::getBirchColor();

	level.getBiomeSource().getBiomeBlock(x, z, 1, 1);
	double temperature = level.getBiomeSource().temperatures[0];
	double downfall = level.getBiomeSource().downfalls[0];
	return FoliageColor::get(temperature, downfall);
}


int_t LeafTile::getItemColor(int_t data)
{
	if ((data & 1) == 1)
		return FoliageColor::getEvergreenColor();
	if ((data & 2) == 2)
		return FoliageColor::getBirchColor();
	return FoliageColor::getDefaultColor();
}

void LeafTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	if (level.isOnline || !level.hasChunksAt(x, y, z, 2))
		return;

	for (int_t dx = -1; dx <= 1; ++dx)
	{
		for (int_t dy = -1; dy <= 1; ++dy)
		{
			for (int_t dz = -1; dz <= 1; ++dz)
			{
				int_t leafX = x + dx;
				int_t leafY = y + dy;
				int_t leafZ = z + dz;
				if (level.getTile(leafX, leafY, leafZ) != Tile::leaves.id)
					continue;

				int_t data = level.getData(leafX, leafY, leafZ);
				level.setDataNoUpdate(leafX, leafY, leafZ, data | CHECK_DECAY_BIT);
			}
		}
	}
}

void LeafTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (level.isOnline)
		return;

	int_t data = level.getData(x, y, z);
	if ((data & CHECK_DECAY_BIT) == 0)
		return;

	constexpr int_t adjacencySize = 32;
	constexpr int_t adjacencyPlane = adjacencySize * adjacencySize;
	constexpr int_t adjacencyCenter = adjacencySize / 2;
	if (adjacentTreeBlocks.empty())
		adjacentTreeBlocks.resize(adjacencySize * adjacencyPlane);
	if (!level.hasChunksAt(x, y, z, REQUIRED_WOOD_RANGE + 1))
		return;

	auto indexAt = [adjacencySize, adjacencyPlane, adjacencyCenter](int_t dx, int_t dy, int_t dz) {
		return (dx + adjacencyCenter) * adjacencyPlane + (dy + adjacencyCenter) * adjacencySize + dz + adjacencyCenter;
	};

	for (int_t dx = -REQUIRED_WOOD_RANGE; dx <= REQUIRED_WOOD_RANGE; ++dx)
	{
		for (int_t dy = -REQUIRED_WOOD_RANGE; dy <= REQUIRED_WOOD_RANGE; ++dy)
		{
			for (int_t dz = -REQUIRED_WOOD_RANGE; dz <= REQUIRED_WOOD_RANGE; ++dz)
			{
				int_t tile = level.getTile(x + dx, y + dy, z + dz);
				int_t &state = adjacentTreeBlocks[indexAt(dx, dy, dz)];
				if (tile == Tile::treeTrunk.id)
					state = 0;
				else if (tile == Tile::leaves.id)
					state = -2;
				else
					state = -1;
			}
		}
	}

	for (int_t distance = 1; distance <= REQUIRED_WOOD_RANGE; ++distance)
	{
		for (int_t dx = -REQUIRED_WOOD_RANGE; dx <= REQUIRED_WOOD_RANGE; ++dx)
		{
			for (int_t dy = -REQUIRED_WOOD_RANGE; dy <= REQUIRED_WOOD_RANGE; ++dy)
			{
				for (int_t dz = -REQUIRED_WOOD_RANGE; dz <= REQUIRED_WOOD_RANGE; ++dz)
				{
					if (adjacentTreeBlocks[indexAt(dx, dy, dz)] != distance - 1)
						continue;

					if (adjacentTreeBlocks[indexAt(dx - 1, dy, dz)] == -2)
						adjacentTreeBlocks[indexAt(dx - 1, dy, dz)] = distance;
					if (adjacentTreeBlocks[indexAt(dx + 1, dy, dz)] == -2)
						adjacentTreeBlocks[indexAt(dx + 1, dy, dz)] = distance;
					if (adjacentTreeBlocks[indexAt(dx, dy - 1, dz)] == -2)
						adjacentTreeBlocks[indexAt(dx, dy - 1, dz)] = distance;
					if (adjacentTreeBlocks[indexAt(dx, dy + 1, dz)] == -2)
						adjacentTreeBlocks[indexAt(dx, dy + 1, dz)] = distance;
					if (adjacentTreeBlocks[indexAt(dx, dy, dz - 1)] == -2)
						adjacentTreeBlocks[indexAt(dx, dy, dz - 1)] = distance;
					if (adjacentTreeBlocks[indexAt(dx, dy, dz + 1)] == -2)
						adjacentTreeBlocks[indexAt(dx, dy, dz + 1)] = distance;
				}
			}
		}
	}

	if (adjacentTreeBlocks[indexAt(0, 0, 0)] >= 0)
		level.setData(x, y, z, data & ~CHECK_DECAY_BIT);
	else
		die(level, x, y, z);
}

void LeafTile::die(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	spawnResources(level, x, y, z, data);
	level.setTile(x, y, z, 0);
}

int_t LeafTile::getResourceCount(Random &random)
{
	return (random.nextInt(20) == 0) ? 1 : 0;
}

int_t LeafTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Tile::sapling.id;
}
int_t LeafTile::getSpawnResourcesAuxValue(int_t data)
{
	return data & LEAF_TYPE_MASK;
}

bool LeafTile::isSolidRender()
{
	return !allowSame;
}

int_t LeafTile::getTexture(Facing face, int_t data)
{
	return ((data & LEAF_TYPE_MASK) == SPRUCE_LEAF) ? (tex + 80) : tex;
}

void LeafTile::setFancy(bool fancy)
{
	allowSame = fancy;
	tex = oTex + (fancy ? 0 : 1);
}

void LeafTile::stepOn(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	Tile::stepOn(level, x, y, z, entity);
}

void LeafTile::harvestBlock(Level &level, Player &player, int_t x, int_t y, int_t z, int_t data)
{
	ItemInstance *selected = player.getSelectedItem();
	if (!level.isOnline && selected != nullptr && selected->itemID == Items::shears->getShiftedIndex())
	{
		if (StatBase *stat = StatList::mineBlockStats[id])
			player.addStat(*stat, 1);
		auto item = std::make_shared<EntityItem>(level, x + 0.5, y + 0.5, z + 0.5, ItemInstance(id, 1, data & 3));
		level.addEntity(item);
	}
	else
	{
		Tile::harvestBlock(level, player, x, y, z, data);
	}
}
