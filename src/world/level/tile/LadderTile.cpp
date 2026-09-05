#include "world/level/tile/LadderTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"

LadderTile::LadderTile(int_t id, int_t tex) : Tile(id, tex, Material::circuits())
{
	updateCachedProperties();
}

void LadderTile::setShapeForData(int_t data)
{
	float thickness = 2.0f / 16.0f;
	if (data == 2)
		setShape(0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f, 1.0f);
	else if (data == 3)
		setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, thickness);
	else if (data == 4)
		setShape(1.0f - thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	else if (data == 5)
		setShape(0.0f, 0.0f, 0.0f, thickness, 1.0f, 1.0f);
}

bool LadderTile::canSurvive(Level &level, int_t x, int_t y, int_t z, int_t data)
{
	if (data == 2) return level.isSolidTile(x, y, z + 1);
	if (data == 3) return level.isSolidTile(x, y, z - 1);
	if (data == 4) return level.isSolidTile(x + 1, y, z);
	if (data == 5) return level.isSolidTile(x - 1, y, z);
	return false;
}

void LadderTile::dropIfUnsupported(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	if (!canSurvive(level, x, y, z, data))
	{
		spawnResources(level, x, y, z, data);
		level.setTile(x, y, z, 0);
	}
}

bool LadderTile::isCubeShaped()
{
	return false;
}

bool LadderTile::isSolidRender()
{
	return false;
}

Tile::Shape LadderTile::getRenderShape()
{
	return SHAPE_LADDER;
}

AABB *LadderTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}

AABB *LadderTile::getTileAABB(Level &level, int_t x, int_t y, int_t z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}

void LadderTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	setShapeForData(level.getData(x, y, z));
}

void LadderTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}


void LadderTile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	int_t data = level.getData(x, y, z);
	if ((data == 0 || face == Facing::NORTH) && level.isSolidTile(x, y, z + 1))
		data = 2;
	if ((data == 0 || face == Facing::SOUTH) && level.isSolidTile(x, y, z - 1))
		data = 3;
	if ((data == 0 || face == Facing::WEST) && level.isSolidTile(x + 1, y, z))
		data = 4;
	if ((data == 0 || face == Facing::EAST) && level.isSolidTile(x - 1, y, z))
		data = 5;
	level.setData(x, y, z, data);
	if (!canSurvive(level, x, y, z, data))
	{
		spawnResources(level, x, y, z, data);
		level.setTile(x, y, z, 0);
	}
}

void LadderTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	dropIfUnsupported(level, x, y, z);
}