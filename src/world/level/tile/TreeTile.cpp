#include "world/level/tile/TreeTile.h"

#include "world/level/Level.h"
#include "world/level/tile/LeafTile.h"

TreeTile::TreeTile(int_t id) : Tile(id, Material::wood)
{
	tex = 20;
}

int_t TreeTile::getResourceCount(Random &random)
{
	return 1;
}

int_t TreeTile::getResource(int_t data, Random &random)
{
	return id;
}

void TreeTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	if (level.isOnline || !level.hasChunksAt(x, y, z, LeafTile::REQUIRED_WOOD_RANGE + 1))
		return;

	for (int_t dx = -LeafTile::REQUIRED_WOOD_RANGE; dx <= LeafTile::REQUIRED_WOOD_RANGE; ++dx)
	{
		for (int_t dy = -LeafTile::REQUIRED_WOOD_RANGE; dy <= LeafTile::REQUIRED_WOOD_RANGE; ++dy)
		{
			for (int_t dz = -LeafTile::REQUIRED_WOOD_RANGE; dz <= LeafTile::REQUIRED_WOOD_RANGE; ++dz)
			{
				int_t leafX = x + dx;
				int_t leafY = y + dy;
				int_t leafZ = z + dz;
				if (level.getTile(leafX, leafY, leafZ) != Tile::leaves.id)
					continue;

				int_t data = level.getData(leafX, leafY, leafZ);
				level.setDataNoUpdate(leafX, leafY, leafZ, data | LeafTile::CHECK_DECAY_BIT);
			}
		}
	}
}

int_t TreeTile::getTexture(Facing face, int_t data)
{
	int_t type = data & TRUNK_TYPE_MASK;
	if (face == Facing::UP || face == Facing::DOWN)
		return 21;
	if (type == SPRUCE_TRUNK)
		return 116;
	if (type == BIRCH_TRUNK)
		return 117;
	return 20;
}

int_t TreeTile::getSpawnResourcesAuxValue(int_t data)
{
	return data;
}