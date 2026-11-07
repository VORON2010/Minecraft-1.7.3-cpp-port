#include "world/level/tile/PumpkinTile.h"
#include "world/entity/animal/MushroomCow.h"
#include "world/level/Level.h"
#include "world/level/tile/SnowBlockTile.h"

#include "world/level/material/Material.h"

PumpkinTile::PumpkinTile(int_t id, int_t tex, bool lit) : Tile(id, tex, Material::pumpkin()), lit(lit)
{
	setTicking(true);
}

int_t PumpkinTile::getTexture(Facing face, int_t data)
{
	if (face == Facing::UP || face == Facing::DOWN)
		return tex;

	int_t frontTexture = tex + 17;
	if (lit)
		frontTexture++;

	if ((data == 2 && face == Facing::NORTH) ||
		(data == 3 && face == Facing::EAST) ||
		(data == 0 && face == Facing::SOUTH) ||
		(data == 1 && face == Facing::WEST))
	{
		return frontTexture;
	}

	return tex + 16;
}
void PumpkinTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onPlace(level, x, y, z);
	if (level.getTile(x, y - 1, z) != Tile::snowBlock.id || level.getTile(x, y - 2, z) != Tile::snowBlock.id)
		return;
	if (!level.isOnline)
	{
		level.setTile(x, y, z, 0);
		level.setTile(x, y - 1, z, 0);
		level.setTile(x, y - 2, z, 0);
		auto golem = std::make_shared<SnowMan>(level);
		golem->moveTo(x + 0.5, y - 1.95, z + 0.5, 0.0f, 0.0f);
		level.addEntity(golem);
	}
	for (int_t i = 0; i < 120; ++i)
		level.addParticle(u"snowshovel", x + level.random.nextDouble(), y - 2 + level.random.nextDouble() * 2.5, z + level.random.nextDouble(), 0.0, 0.0, 0.0);
}
