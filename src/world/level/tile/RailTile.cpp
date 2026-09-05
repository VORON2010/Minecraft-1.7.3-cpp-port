#include "world/level/tile/RailTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/RailLogic.h"

namespace
{
	constexpr int_t RAIL_FLAT_HEIGHT_TEXELS = 2;
	constexpr int_t RAIL_ASCEND_HEIGHT_TEXELS = 10;
	constexpr int_t RAIL_POWERED_BIT = 8;

	int_t getRailShape(int_t data, bool poweredRail)
	{
		return poweredRail ? (data & 7) : data;
	}

	bool checkPoweredRail(const RailTile &rail, Level &level, int_t x, int_t y, int_t z, bool forward, int_t depth, int_t axis);

	bool findPoweredRailSignal(const RailTile &rail, Level &level, int_t x, int_t y, int_t z, int_t data, bool forward, int_t depth)
	{
		if (depth >= 8)
			return false;

		int_t shape = data & 7;
		bool checkBelow = true;
		int_t axis = shape == 0 || shape == 4 || shape == 5 ? 0 : 1;

		switch (shape)
		{
		case 0:
			if (forward)
				z++;
			else
				z--;
			break;
		case 1:
			if (forward)
				x--;
			else
				x++;
			break;
		case 2:
			if (forward)
				x--;
			else
			{
				x++;
				y++;
				checkBelow = false;
			}
			axis = 1;
			break;
		case 3:
			if (forward)
			{
				x--;
				y++;
				checkBelow = false;
			}
			else
			{
				x++;
			}
			axis = 1;
			break;
		case 4:
			if (forward)
				z++;
			else
			{
				z--;
				y++;
				checkBelow = false;
			}
			axis = 0;
			break;
		case 5:
			if (forward)
			{
				z++;
				y++;
				checkBelow = false;
			}
			else
			{
				z--;
			}
			axis = 0;
			break;
		default:
			break;
		}

		if (checkPoweredRail(rail, level, x, y, z, forward, depth, axis))
			return true;
		return checkBelow && checkPoweredRail(rail, level, x, y - 1, z, forward, depth, axis);
	}

	bool checkPoweredRail(const RailTile &rail, Level &level, int_t x, int_t y, int_t z, bool forward, int_t depth, int_t axis)
	{
		(void)rail;
		int_t tileId = level.getTile(x, y, z);
		if (tileId != 27)
			return false;

		int_t data = level.getData(x, y, z);
		int_t shape = data & 7;
		if (axis == 1 && (shape == 0 || shape == 4 || shape == 5))
			return false;
		if (axis == 0 && (shape == 1 || shape == 2 || shape == 3))
			return false;

		if ((data & RAIL_POWERED_BIT) == 0)
			return false;

		if (!level.hasNeighborSignal(x, y, z) && !level.hasNeighborSignal(x, y + 1, z))
			return findPoweredRailSignal(rail, level, x, y, z, data, forward, depth + 1);
		return true;
	}
}

RailTile::RailTile(int_t id, int_t tex, bool poweredRail)
	: Tile(id, tex, Material::circuits()), poweredRail(poweredRail)
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, static_cast<float>(RAIL_FLAT_HEIGHT_TEXELS) / 16.0f, 1.0f);
	updateCachedProperties();
}

bool RailTile::isRail(LevelSource &level, int_t x, int_t y, int_t z)
{
	return isRail(level.getTile(x, y, z));
}

bool RailTile::isRail(int_t tileId)
{
	return tileId == 27 || tileId == 28 || tileId == 66;
}

AABB *RailTile::getTileAABB(Level &level, int_t x, int_t y, int_t z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}

void RailTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t shape = getRailShape(level.getData(x, y, z), poweredRail);
	float height = (shape >= 2 && shape <= 5) ? static_cast<float>(RAIL_ASCEND_HEIGHT_TEXELS) / 16.0f : static_cast<float>(RAIL_FLAT_HEIGHT_TEXELS) / 16.0f;
	setShape(0.0f, 0.0f, 0.0f, 1.0f, height, 1.0f);
}

void RailTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, static_cast<float>(RAIL_FLAT_HEIGHT_TEXELS) / 16.0f, 1.0f);
}

int_t RailTile::getTexture(Facing face, int_t data)
{
	(void)face;
	if (poweredRail)
	{
		if (id == 27 && (data & RAIL_POWERED_BIT) == 0)
			return tex - 16;
		return tex;
	}

	return data >= 6 ? tex - 16 : tex;
}

bool RailTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	return level.isBlockNormalCube(x, y - 1, z);
}

void RailTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (!level.isOnline)
		updateRail(level, x, y, z, true);
}

void RailTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	int_t data = level.getData(x, y, z);
	int_t shape = getRailShape(data, poweredRail);
	if (!isSupported(level, x, y, z, shape))
	{
		spawnResources(level, x, y, z, data);
		level.setTile(x, y, z, 0);
		return;
	}

	if (id == 27)
	{
		updatePoweredState(level, x, y, z, data);
		return;
	}

	if (!poweredRail && tile > 0 && Tile::tiles[tile] != nullptr && Tile::tiles[tile]->isSignalSource()
		&& RailLogic::getAdjacentTrackCount(*this, level, x, y, z) == 3)
	{
		updateRail(level, x, y, z, false);
	}
}

void RailTile::updateRail(Level &level, int_t x, int_t y, int_t z, bool forceUpdate)
{
	if (level.isOnline)
		return;

	RailLogic(*this, level, x, y, z).place(level.hasNeighborSignal(x, y, z), forceUpdate);
}

bool RailTile::updatePoweredState(Level &level, int_t x, int_t y, int_t z, int_t data)
{
	bool powered = level.hasNeighborSignal(x, y, z)
		|| level.hasNeighborSignal(x, y + 1, z)
		|| findPoweredRailSignal(*this, level, x, y, z, data, true, 0)
		|| findPoweredRailSignal(*this, level, x, y, z, data, false, 0);

	int_t shape = data & 7;
	bool changed = false;
	if (powered && (data & RAIL_POWERED_BIT) == 0)
	{
		setRailData(level, x, y, z, shape | RAIL_POWERED_BIT);
		changed = true;
	}
	else if (!powered && (data & RAIL_POWERED_BIT) != 0)
	{
		setRailData(level, x, y, z, shape);
		changed = true;
	}

	if (changed)
	{
		level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
		if (shape >= 2 && shape <= 5)
			level.notifyBlocksOfNeighborChange(x, y + 1, z, id);
	}

	return changed;
}

bool RailTile::isSupported(Level &level, int_t x, int_t y, int_t z, int_t shapeData) const
{
	if (!level.isBlockNormalCube(x, y - 1, z))
		return false;
	if (shapeData == 2)
		return level.isBlockNormalCube(x + 1, y, z);
	if (shapeData == 3)
		return level.isBlockNormalCube(x - 1, y, z);
	if (shapeData == 4)
		return level.isBlockNormalCube(x, y, z - 1);
	if (shapeData == 5)
		return level.isBlockNormalCube(x, y, z + 1);
	return true;
}

void RailTile::setRailData(Level &level, int_t x, int_t y, int_t z, int_t data) const
{
	if (level.setDataNoUpdate(x, y, z, data))
		level.setTilesDirty(x, y, z, x, y, z);
}
