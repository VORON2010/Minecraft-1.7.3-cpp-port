#include "world/level/tile/LeverTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"



LeverTile::LeverTile(int_t id, int_t tex) : Tile(id, tex, Material::circuits())
{
	setDestroyTime(0.5f);
	updateCachedProperties();
}

bool LeverTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	
	for (int_t face = 1; face <= 5; face++)
	{
		if (canPlaceOnSide(level, x, y, z, static_cast<Facing>(face)))
			return true;
	}
	return false;
}

bool LeverTile::canPlaceOnSide(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	
	if (face == Facing::UP)
		return level.isBlockNormalCube(x, y - 1, z);
	if (face == Facing::NORTH)
		return level.isBlockNormalCube(x, y, z + 1);
	if (face == Facing::SOUTH)
		return level.isBlockNormalCube(x, y, z - 1);
	if (face == Facing::WEST)
		return level.isBlockNormalCube(x + 1, y, z);
	if (face == Facing::EAST)
		return level.isBlockNormalCube(x - 1, y, z);
	return false;
}

void LeverTile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	
	int_t orient = 0;

	if (face == Facing::UP && canPlaceOnSide(level, x, y, z, Facing::UP))
		orient = 5 + level.random.nextInt(2);
	else if (face == Facing::NORTH && canPlaceOnSide(level, x, y, z, Facing::NORTH))
		orient = 4;
	else if (face == Facing::SOUTH && canPlaceOnSide(level, x, y, z, Facing::SOUTH))
		orient = 3;
	else if (face == Facing::WEST && canPlaceOnSide(level, x, y, z, Facing::WEST))
		orient = 2;
	else if (face == Facing::EAST && canPlaceOnSide(level, x, y, z, Facing::EAST))
		orient = 1;

	if (orient == 0)
	{
		
		orient = 1;
		bool found = false;
		for (int_t f = 1; f <= 5 && !found; f++)
		{
			if (canPlaceOnSide(level, x, y, z, static_cast<Facing>(f)))
			{
				found = true;
				if (f == 1) orient = 5 + level.random.nextInt(2);
				else if (f == 2) orient = 4;
				else if (f == 3) orient = 3;
				else if (f == 4) orient = 2;
				else orient = 1;
			}
		}
		if (!found)
		{
			spawnResources(level, x, y, z, level.getData(x, y, z));
			level.setTile(x, y, z, 0);
			return;
		}
	}

	int_t data = level.getData(x, y, z);
	int_t powered = data & 8;
	level.setData(x, y, z, orient | powered);
}

void LeverTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (level.isBlockNormalCube(x - 1, y, z))
		level.setData(x, y, z, 1);
	else if (level.isBlockNormalCube(x + 1, y, z))
		level.setData(x, y, z, 2);
	else if (level.isBlockNormalCube(x, y, z - 1))
		level.setData(x, y, z, 3);
	else if (level.isBlockNormalCube(x, y, z + 1))
		level.setData(x, y, z, 4);
	else if (level.isBlockNormalCube(x, y - 1, z))
		level.setData(x, y, z, 5 + level.random.nextInt(2));

	checkCanSurvive(level, x, y, z);
}

bool LeverTile::checkCanSurvive(Level &level, int_t x, int_t y, int_t z)
{
	if (!mayPlace(level, x, y, z))
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
		return false;
	}
	return true;
}

void LeverTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;

	if (checkCanSurvive(level, x, y, z))
	{
		int_t data = level.getData(x, y, z);
		int_t orient = data & 7;
		bool supported = false;

		if (orient == 1 && level.isBlockNormalCube(x - 1, y, z))
			supported = true;
		else if (orient == 2 && level.isBlockNormalCube(x + 1, y, z))
			supported = true;
		else if (orient == 3 && level.isBlockNormalCube(x, y, z - 1))
			supported = true;
		else if (orient == 4 && level.isBlockNormalCube(x, y, z + 1))
			supported = true;
		else if (orient == 5 && level.isBlockNormalCube(x, y - 1, z))
			supported = true;
		else if (orient == 6 && level.isBlockNormalCube(x, y - 1, z))
			supported = true;

		if (!supported)
		{
			spawnResources(level, x, y, z, data);
			level.setTile(x, y, z, 0);
		}
	}
}

bool LeverTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	if (level.isOnline)
		return true;

	int_t data = level.getData(x, y, z);
	int_t orient = data & 7;
	int_t powered = 8 - (data & 8);
	level.setData(x, y, z, orient | powered);
	level.setTilesDirty(x, y, z, x, y, z);
	level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, u"random.click", 0.3f, powered > 0 ? 0.6f : 0.5f);
	level.notifyBlocksOfNeighborChange(x, y, z, id);
	if (orient == 1) level.notifyBlocksOfNeighborChange(x - 1, y, z, id);
	else if (orient == 2) level.notifyBlocksOfNeighborChange(x + 1, y, z, id);
	else if (orient == 3) level.notifyBlocksOfNeighborChange(x, y, z - 1, id);
	else if (orient == 4) level.notifyBlocksOfNeighborChange(x, y, z + 1, id);
	else level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
	return true;
}

void LeverTile::attack(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	
	use(level, x, y, z, player);
}

bool LeverTile::getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	(void)dir;
	
	return (level.getData(x, y, z) & 8) > 0;
}

bool LeverTile::getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	
	int_t data = level.getData(x, y, z);
	if ((data & 8) == 0)
		return false;

	
	
	int_t orient = data & 7;
	if (orient == 5 && dir == 1) return true;
	if (orient == 4 && dir == 2) return true;
	if (orient == 3 && dir == 3) return true;
	if (orient == 2 && dir == 4) return true;
	if (orient == 1 && dir == 5) return true;
	return false;
}

void LeverTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	
	if ((level.getData(x, y, z) & 8) > 0)
	{
		level.notifyBlocksOfNeighborChange(x, y, z, id);
		int_t orient = level.getData(x, y, z) & 7;
		if (orient == 1) level.notifyBlocksOfNeighborChange(x - 1, y, z, id);
		else if (orient == 2) level.notifyBlocksOfNeighborChange(x + 1, y, z, id);
		else if (orient == 3) level.notifyBlocksOfNeighborChange(x, y, z - 1, id);
		else if (orient == 4) level.notifyBlocksOfNeighborChange(x, y, z + 1, id);
		else level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
	}
}

void LeverTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	
	int_t orient = level.getData(x, y, z) & 7;
	float f = 0.1875f;

	if (orient == 1)
		setShape(0.0f, 0.2f, 0.5f - f, f * 2.0f, 0.8f, 0.5f + f);
	else if (orient == 2)
		setShape(1.0f - f * 2.0f, 0.2f, 0.5f - f, 1.0f, 0.8f, 0.5f + f);
	else if (orient == 3)
		setShape(0.5f - f, 0.2f, 0.0f, 0.5f + f, 0.8f, f * 2.0f);
	else if (orient == 4)
		setShape(0.5f - f, 0.2f, 1.0f - f * 2.0f, 0.5f + f, 0.8f, 1.0f);
	else if (orient == 5 || orient == 6)
		setShape(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, 0.6f, 0.5f + f);
}