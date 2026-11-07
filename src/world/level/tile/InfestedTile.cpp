#include "world/level/tile/InfestedTile.h"

#include "world/entity/monster/Silverfish.h"
#include "world/level/Level.h"
#include "world/level/tile/StoneBrickTile.h"
#include "world/level/tile/StoneTile.h"

InfestedTile::InfestedTile(int_t id) : Tile(id, 1, Material::clay)
{
	updateCachedProperties();
}

int_t InfestedTile::getTexture(Facing face, int_t data)
{
	(void)face;
	if (data == 1)
		return Tile::cobblestone.tex;
	if (data == 2)
		return Tile::stoneBrick.tex;
	return Tile::rock.tex;
}

void InfestedTile::destroy(Level &level, int_t x, int_t y, int_t z, int_t data)
{
	if (!level.isOnline)
	{
		auto silverfish = std::make_shared<Silverfish>(level);
		silverfish->moveTo(x + 0.5, y, z + 0.5, 0.0f, 0.0f);
		level.addEntity(silverfish);
		silverfish->spawnAnim();
	}
	Tile::destroy(level, x, y, z, data);
}

int_t InfestedTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

bool InfestedTile::canBeInfested(int_t tile)
{
	return tile == Tile::rock.id || tile == Tile::cobblestone.id || tile == Tile::stoneBrick.id;
}

int_t InfestedTile::getVariant(int_t tile)
{
	if (tile == Tile::cobblestone.id)
		return 1;
	return tile == Tile::stoneBrick.id ? 2 : 0;
}
