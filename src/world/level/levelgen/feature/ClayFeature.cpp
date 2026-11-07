#include "world/level/levelgen/feature/ClayFeature.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/ClayTile.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"

ClayFeature::ClayFeature(int_t count) : count(count)
{
}

// 1.0: a flat disc of clay replacing dirt under water
bool ClayFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	if (&level.getMaterial(x, y, z) != static_cast<const Material *>(&Material::water))
		return false;

	int_t r = random.nextInt(count - 2) + 2;
	int_t yr = 1;
	for (int_t xx = x - r; xx <= x + r; ++xx)
	{
		for (int_t zz = z - r; zz <= z + r; ++zz)
		{
			int_t dx = xx - x;
			int_t dz = zz - z;
			if (dx * dx + dz * dz > r * r)
				continue;
			for (int_t yy = y - yr; yy <= y + yr; ++yy)
			{
				int_t t = level.getTile(xx, yy, zz);
				if (t == Tile::dirt.id || t == Tile::clay.id)
					level.setTileNoUpdate(xx, yy, zz, Tile::clay.id);
			}
		}
	}
	return true;
}
