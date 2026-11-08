// 1.5.2 canWalkThrough for the blocks that override it, and DoorBlock.setOpen
#include "world/level/tile/Tile.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/FenceTile.h"
#include "world/level/tile/FenceGateTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/PressurePlateTile.h"
#include "world/level/tile/SignTile.h"
#include "world/level/tile/TrapDoorTile.h"
#include "world/level/Level.h"
#include "world/level/LevelSource.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"

bool Tile::isPathfindable(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return !material.blocksMotion();
}

bool DoorTile::isPathfindable(LevelSource &level, int_t x, int_t y, int_t z)
{
	// beta door data keeps the open bit in both halves
	return (level.getData(x, y, z) & 4) != 0;
}

bool FenceTile::isPathfindable(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return false;
}

bool FenceGateTile::isPathfindable(LevelSource &level, int_t x, int_t y, int_t z)
{
	return isOpen(level.getData(x, y, z));
}

bool LiquidTile::isPathfindable(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return &material != &Material::lava;
}

bool PressurePlateTile::isPathfindable(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return true;
}

bool SignTile::isPathfindable(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return true;
}

bool TrapDoorTile::isPathfindable(LevelSource &level, int_t x, int_t y, int_t z)
{
	return !isOpen(level.getData(x, y, z));
}

void DoorTile::setOpen(Level &level, int_t x, int_t y, int_t z, bool open)
{
	int_t data = level.getData(x, y, z);
	if ((data & 8) != 0)
	{
		if (level.getTile(x, y - 1, z) == id)
			setOpen(level, x, y - 1, z, open);
		return;
	}
	bool isOpen = (data & 4) != 0;
	if (isOpen == open)
		return;
	int_t newData = data ^ 4;
	level.noNeighborUpdate = true;
	level.setDataNoUpdate(x, y, z, newData);
	if (level.getTile(x, y + 1, z) == id)
		level.setDataNoUpdate(x, y + 1, z, newData + 8);
	level.noNeighborUpdate = false;
	level.tileUpdated(x, y, z, id);
	if (level.getTile(x, y + 1, z) == id)
		level.tileUpdated(x, y + 1, z, id);
	level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, open ? u"random.door_open" : u"random.door_close", 1.0f, level.random.nextFloat() * 0.1f + 0.9f);
}
