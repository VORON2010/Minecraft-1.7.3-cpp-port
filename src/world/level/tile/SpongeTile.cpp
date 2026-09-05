#include "world/level/tile/SpongeTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"

SpongeTile::SpongeTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
}

void SpongeTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onPlace(level, x, y, z);
	for (int dx = -2; dx <= 2; dx++)
	{
		for (int dy = -2; dy <= 2; dy++)
		{
			for (int dz = -2; dz <= 2; dz++)
			{
				(void)level.getMaterial(x + dx, y + dy, z + dz);
			}
		}
	}
}

void SpongeTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	for (int dx = -2; dx <= 2; dx++)
	{
		for (int dy = -2; dy <= 2; dy++)
		{
			for (int dz = -2; dz <= 2; dz++)
			{
				level.updateNeighborsAt(x + dx, y + dy, z + dz, level.getTile(x + dx, y + dy, z + dz));
			}
		}
	}
}