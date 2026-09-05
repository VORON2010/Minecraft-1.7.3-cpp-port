#include "world/item/Item.h"
#include "world/level/tile/RedstoneOreTile.h"
#include "world/item/Items.h"
#include "world/entity/Entity.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"

RedstoneOreTile::RedstoneOreTile(int_t id, int_t tex, bool glowing) : Tile(id, tex, Material::stone), glowing(glowing)
{
	if (glowing)
	{
		setLightEmission(9);
		setTicking(true);
	}
}

int_t RedstoneOreTile::getTickDelay()
{
	return 30;
}

int_t RedstoneOreTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Items::redstone->getShiftedIndex();
}

int_t RedstoneOreTile::getResourceCount(Random &random)
{
	return 4 + random.nextInt(2);
}

void RedstoneOreTile::attack(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	interact(level, x, y, z);
	Tile::attack(level, x, y, z, player);
}

void RedstoneOreTile::stepOn(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	interact(level, x, y, z);
	Tile::stepOn(level, x, y, z, entity);
}

bool RedstoneOreTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	interact(level, x, y, z);
	return Tile::use(level, x, y, z, player);
}

void RedstoneOreTile::interact(Level &level, int_t x, int_t y, int_t z)
{
	poofParticles(level, x, y, z);
	if (!glowing)
		level.setTile(x, y, z, Tile::redstoneOreGlowing.id);
}

void RedstoneOreTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;
	if (glowing)
		level.setTile(x, y, z, Tile::redstoneOre.id);
}

void RedstoneOreTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;
	if (glowing)
		poofParticles(level, x, y, z);
}

void RedstoneOreTile::poofParticles(Level &level, int_t x, int_t y, int_t z)
{
	Random &random = level.random;
	const double offset = 0.0625;

	for (int_t face = 0; face < 6; ++face)
	{
		double px = static_cast<double>(x) + random.nextFloat();
		double py = static_cast<double>(y) + random.nextFloat();
		double pz = static_cast<double>(z) + random.nextFloat();

		if (face == 0 && !level.isSolidTile(x, y + 1, z)) py = static_cast<double>(y) + 1.0 + offset;
		if (face == 1 && !level.isSolidTile(x, y - 1, z)) py = static_cast<double>(y) - offset;
		if (face == 2 && !level.isSolidTile(x, y, z + 1)) pz = static_cast<double>(z) + 1.0 + offset;
		if (face == 3 && !level.isSolidTile(x, y, z - 1)) pz = static_cast<double>(z) - offset;
		if (face == 4 && !level.isSolidTile(x + 1, y, z)) px = static_cast<double>(x) + 1.0 + offset;
		if (face == 5 && !level.isSolidTile(x - 1, y, z)) px = static_cast<double>(x) - offset;

		if (px < x || px > x + 1 || py < y || py > y + 1 || pz < z || pz > z + 1)
			level.addParticle(u"reddust", px, py, pz, 0.0, 0.0, 0.0);
	}
}