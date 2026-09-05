#include "world/level/tile/TrapDoorTile.h"

#include "world/level/Level.h"

TrapDoorTile::TrapDoorTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
	updateCachedProperties();
}

bool TrapDoorTile::isOpen(int_t data)
{
	return (data & 4) != 0;
}

void TrapDoorTile::setShapeForData(int_t data)
{
	float thickness = 3.0f / 16.0f;
	setShape(0.0f, 0.0f, 0.0f, 1.0f, thickness, 1.0f);
	if (isOpen(data))
	{
		if ((data & 3) == 0)
			setShape(0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f, 1.0f);
		if ((data & 3) == 1)
			setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, thickness);
		if ((data & 3) == 2)
			setShape(1.0f - thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		if ((data & 3) == 3)
			setShape(0.0f, 0.0f, 0.0f, thickness, 1.0f, 1.0f);
	}
}

bool TrapDoorTile::canSurvive(Level &level, int_t x, int_t y, int_t z, int_t data)
{
	int_t sx = x;
	int_t sz = z;
	if ((data & 3) == 0) sz = z + 1;
	if ((data & 3) == 1) sz = z - 1;
	if ((data & 3) == 2) sx = x + 1;
	if ((data & 3) == 3) sx = x - 1;
	return level.isSolidTile(sx, y, sz);
}

void TrapDoorTile::dropIfUnsupported(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	if (!canSurvive(level, x, y, z, data))
	{
		spawnResources(level, x, y, z, data);
		level.setTile(x, y, z, 0);
	}
}

bool TrapDoorTile::isCubeShaped()
{
	return false;
}

bool TrapDoorTile::isSolidRender()
{
	return false;
}

AABB *TrapDoorTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}

AABB *TrapDoorTile::getTileAABB(Level &level, int_t x, int_t y, int_t z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}

void TrapDoorTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	setShapeForData(level.getData(x, y, z));
}

void TrapDoorTile::updateDefaultShape()
{
	float thickness = 3.0f / 16.0f;
	setShape(0.0f, 0.0f, 0.0f, 1.0f, thickness, 1.0f);
}

bool TrapDoorTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	level.setData(x, y, z, level.getData(x, y, z) ^ 4);
	level.levelEvent(&player, 1003, x, y, z, 0);
	return true;
}

void TrapDoorTile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	int_t data = -1;
	if (face == Facing::NORTH)
		data = 0;
	if (face == Facing::SOUTH)
		data = 1;
	if (face == Facing::WEST)
		data = 2;
	if (face == Facing::EAST)
		data = 3;
	if (data >= 0)
		level.setData(x, y, z, data);
	if (data < 0 || !canSurvive(level, x, y, z, data))
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
	}
}

void TrapDoorTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	dropIfUnsupported(level, x, y, z);

	if (tile > 0 && Tile::tiles[tile] != nullptr && Tile::tiles[tile]->isSignalSource())
	{
		bool powered = level.hasNeighborSignal(x, y, z);
		int_t data = level.getData(x, y, z);
		bool isOpen = (data & 4) != 0;
		if (isOpen != powered)
		{
			level.setData(x, y, z, data ^ 4);
			level.levelEvent(nullptr, 1003, x, y, z, 0);
		}
	}
}