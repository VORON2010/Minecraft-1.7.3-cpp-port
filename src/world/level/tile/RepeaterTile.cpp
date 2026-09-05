#include "world/level/tile/RepeaterTile.h"

#include "util/Mth.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/RedStoneDustTile.h"

namespace
{
	int_t getDelaySetting(int_t data)
	{
		return (data & 12) >> 2;
	}
}

RepeaterTile::RepeaterTile(int_t id, bool repeaterPowered)
	: Tile(id, 6, Material::circuits()), repeaterPowered(repeaterPowered)
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 2.0f / 16.0f, 1.0f);
	updateCachedProperties();
}

bool RepeaterTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	return level.isBlockNormalCube(x, y - 1, z);
}

bool RepeaterTile::canSurvive(Level &level, int_t x, int_t y, int_t z) const
{
	return level.isBlockNormalCube(x, y - 1, z);
}

void RepeaterTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;

	int_t data = level.getData(x, y, z);
	bool powered = hasInputSignal(level, x, y, z, data);
	if (repeaterPowered)
	{
		if (!powered)
			level.setTileAndData(x, y, z, Tile::repeaterIdle.id, data);
	}
	else
	{
		if (powered)
		{
			level.setTileAndData(x, y, z, Tile::repeaterActive.id, data);
			if (!hasInputSignal(level, x, y, z, data))
				level.scheduleBlockUpdate(x, y, z, Tile::repeaterActive.id, getDelayTicks(data));
		}
	}
}

int_t RepeaterTile::getTexture(Facing face, int_t data)
{
	(void)data;
	if (face == Facing::DOWN)
		return repeaterPowered ? 99 : 115;
	if (face == Facing::UP)
		return repeaterPowered ? 147 : 131;
	return 5;
}

bool RepeaterTile::shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	if (face == Facing::DOWN || face == Facing::UP)
		return false;
	return Tile::shouldRenderFace(level, x, y, z, face);
}

bool RepeaterTile::getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	return getSignal(level, x, y, z, dir);
}

bool RepeaterTile::getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	if (!repeaterPowered)
		return false;
	return getOutputDirection(level.getData(x, y, z)) == dir;
}

void RepeaterTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	if (!canSurvive(level, x, y, z))
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
		return;
	}

	int_t data = level.getData(x, y, z);
	bool powered = hasInputSignal(level, x, y, z, data);
	int_t delay = getDelayTicks(data);
	if (repeaterPowered)
	{
		if (!powered)
			level.scheduleBlockUpdate(x, y, z, id, delay);
	}
	else if (powered)
	{
		level.scheduleBlockUpdate(x, y, z, id, delay);
	}
}

bool RepeaterTile::hasInputSignal(Level &level, int_t x, int_t y, int_t z, int_t data) const
{
	switch (data & 3)
	{
	case 0:
		return level.getSignal(x, y, z + 1, 3)
			|| (level.getTile(x, y, z + 1) == Tile::redstoneWire.id && level.getData(x, y, z + 1) > 0);
	case 1:
		return level.getSignal(x - 1, y, z, 4)
			|| (level.getTile(x - 1, y, z) == Tile::redstoneWire.id && level.getData(x - 1, y, z) > 0);
	case 2:
		return level.getSignal(x, y, z - 1, 2)
			|| (level.getTile(x, y, z - 1) == Tile::redstoneWire.id && level.getData(x, y, z - 1) > 0);
	case 3:
		return level.getSignal(x + 1, y, z, 5)
			|| (level.getTile(x + 1, y, z) == Tile::redstoneWire.id && level.getData(x + 1, y, z) > 0);
	default:
		return false;
	}
}

bool RepeaterTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	int_t data = level.getData(x, y, z);
	int_t delayBits = ((data & 12) + 4) & 12;
	level.setData(x, y, z, (data & 3) | delayBits);
	return true;
}

void RepeaterTile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t rotation = ((Mth::floor(static_cast<double>(player.yRot) * 4.0 / 360.0 + 0.5) & 3) + 2) % 4;
	int_t data = (level.getData(x, y, z) & 12) | rotation;
	level.setData(x, y, z, data);
	if (hasInputSignal(level, x, y, z, data))
		level.scheduleBlockUpdate(x, y, z, id, 1);
}

void RepeaterTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.notifyBlocksOfNeighborChange(x + 1, y, z, id);
	level.notifyBlocksOfNeighborChange(x - 1, y, z, id);
	level.notifyBlocksOfNeighborChange(x, y, z + 1, id);
	level.notifyBlocksOfNeighborChange(x, y, z - 1, id);
	level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
	level.notifyBlocksOfNeighborChange(x, y + 1, z, id);
}

void RepeaterTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 2.0f / 16.0f, 1.0f);
}

void RepeaterTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 2.0f / 16.0f, 1.0f);
}

int_t RepeaterTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Items::redstoneRepeater->getShiftedIndex();
}

void RepeaterTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (!repeaterPowered)
		return;

	int_t data = level.getData(x, y, z);
	double px = static_cast<double>(x) + 0.5 + (random.nextFloat() - 0.5f) * 0.2;
	double py = static_cast<double>(y) + 0.4 + (random.nextFloat() - 0.5f) * 0.2;
	double pz = static_cast<double>(z) + 0.5 + (random.nextFloat() - 0.5f) * 0.2;
	double offX = 0.0;
	double offZ = 0.0;
	if (random.nextInt(2) == 0)
	{
		switch (data & 3)
		{
		case 0: offZ = -0.3125; break;
		case 1: offX = 0.3125; break;
		case 2: offZ = 0.3125; break;
		case 3: offX = -0.3125; break;
		}
	}
	else
	{
		double delayOffset = getDelayTorchOffset(getDelaySetting(data));
		switch (data & 3)
		{
		case 0: offZ = delayOffset; break;
		case 1: offX = -delayOffset; break;
		case 2: offZ = -delayOffset; break;
		case 3: offX = delayOffset; break;
		}
	}

	level.addParticle(u"reddust", px + offX, py, pz + offZ, 0.0, 0.0, 0.0);
}

int_t RepeaterTile::getOutputDirection(int_t data)
{
	switch (data & 3)
	{
	case 0: return 3;
	case 1: return 4;
	case 2: return 2;
	case 3: return 5;
	default: return -1;
	}
}

int_t RepeaterTile::getWireConnectionDirection(int_t data)
{
	switch (data & 3)
	{
	case 0: return 2;
	case 1: return 3;
	case 2: return 0;
	case 3: return 1;
	default: return -1;
	}
}

double RepeaterTile::getDelayTorchOffset(int_t delaySetting)
{
	switch (delaySetting & 3)
	{
	case 0: return -0.0625;
	case 1: return 1.0 / 16.0;
	case 2: return 0.1875;
	default: return 0.3125;
	}
}

int_t RepeaterTile::getDelayTicks(int_t data)
{
	static const int_t delaySteps[] = {2, 4, 6, 8};
	return delaySteps[getDelaySetting(data)];
}
