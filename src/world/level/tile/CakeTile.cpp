#include "world/level/tile/CakeTile.h"

#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/LevelSource.h"

CakeTile::CakeTile(int_t id, int_t tex) : Tile(id, tex, Material::cakeMaterial)
{
	setTicking(true);
	updateCachedProperties();
}

void CakeTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	float s = 1.0f / 16.0f;
	float left = (1 + data * 2) / 16.0f;
	float top = 0.5f;
	setShape(left, 0.0f, s, 1.0f - s, top, 1.0f - s);
}

void CakeTile::updateDefaultShape()
{
	float s = 1.0f / 16.0f;
	float top = 0.5f;
	setShape(s, 0.0f, s, 1.0f - s, top, 1.0f - s);
}

AABB *CakeTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	float s = 1.0f / 16.0f;
	float left = (1 + data * 2) / 16.0f;
	float top = 0.5f;
	return AABB::newTemp(x + left, y, z + s, x + 1.0f - s, y + top - s, z + 1.0f - s);
}

AABB *CakeTile::getTileAABB(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	float s = 1.0f / 16.0f;
	float left = (1 + data * 2) / 16.0f;
	float top = 0.5f;
	return AABB::newTemp(x + left, y, z + s, x + 1.0f - s, y + top, z + 1.0f - s);
}

int_t CakeTile::getTexture(Facing face, int_t data)
{
	if (face == Facing::UP)
		return tex;
	if (face == Facing::DOWN)
		return tex + 3;
	if (data > 0 && face == Facing::WEST)
		return tex + 2;
	return tex + 1;
}

int_t CakeTile::getTexture(Facing face)
{
	if (face == Facing::UP)
		return tex;
	if (face == Facing::DOWN)
		return tex + 3;
	return tex + 1;
}

bool CakeTile::isCubeShaped()
{
	return false;
}

bool CakeTile::isSolidRender()
{
	return false;
}

bool CakeTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	eat(level, x, y, z, player);
	return true;
}

void CakeTile::attack(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	eat(level, x, y, z, player);
}

void CakeTile::eat(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (player.health >= 20)
		return;
	player.heal(3);
	int_t data = level.getData(x, y, z) + 1;
	if (data >= 6)
	{
		level.setTile(x, y, z, 0);
	}
	else
	{
		level.setData(x, y, z, data);
		level.setTileDirty(x, y, z);
	}
}

bool CakeTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (!Tile::mayPlace(level, x, y, z))
		return false;
	return canSurvive(level, x, y, z);
}

void CakeTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	if (!canSurvive(level, x, y, z))
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
	}
}

bool CakeTile::canSurvive(Level &level, int_t x, int_t y, int_t z)
{
	return level.getMaterial(x, y - 1, z).isSolid();
}

int_t CakeTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

int_t CakeTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return 0;
}
