#include "world/level/tile/SandTile.h"

#include "util/Memory.h"
#include "world/entity/item/FallingTile.h"
#include "world/level/Level.h"
#include "world/level/material/LiquidMaterial.h"

namespace
{
	constexpr int_t FIRE_TILE_ID = 51;
	constexpr int_t FALLING_TILE_CHUNK_RANGE = 32;
}

bool SandTile::fallInstantly = false;

SandTile::SandTile(int_t id, int_t tex) : Tile(id, tex, Material::sand)
{
}

void SandTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	checkSlide(level, x, y, z);
}

void SandTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	level.addToTickNextTick(x, y, z, id);
}

int_t SandTile::getTickDelay()
{
	return 3;
}

void SandTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.addToTickNextTick(x, y, z, id);
}

void SandTile::checkSlide(Level &level, int_t x, int_t y, int_t z)
{
	if (!isFree(level, x, y - 1, z) || y < 0)
		return;

	if (!fallInstantly && level.hasChunksAt(x - FALLING_TILE_CHUNK_RANGE, y - FALLING_TILE_CHUNK_RANGE, z - FALLING_TILE_CHUNK_RANGE, x + FALLING_TILE_CHUNK_RANGE, y + FALLING_TILE_CHUNK_RANGE, z + FALLING_TILE_CHUNK_RANGE))
	{
		level.addEntity(Util::make_shared<FallingTile>(level, static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, id));
		return;
	}

	level.setTile(x, y, z, 0);

	while (isFree(level, x, y - 1, z) && y > 0)
		--y;

	if (y > 0)
		level.setTile(x, y, z, id);
}

bool SandTile::isFree(Level &level, int_t x, int_t y, int_t z)
{
	int_t tile = level.getTile(x, y, z);
	if (tile == 0 || tile == FIRE_TILE_ID)
		return true;

	Tile *target = Tile::tiles[tile];
	if (target == nullptr)
		return false;

	const Material &material = target->material;
	const Material &water = Material::water;
	const Material &lava = Material::lava;
	return &material == &water || &material == &lava;
}