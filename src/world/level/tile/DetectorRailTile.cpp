#include "world/level/tile/DetectorRailTile.h"

#include "world/entity/item/EntityMinecart.h"
#include "world/level/Level.h"
#include "world/phys/AABB.h"

DetectorRailTile::DetectorRailTile(int_t id, int_t tex) : RailTile(id, tex, true)
{
	setTicking(true);
}

bool DetectorRailTile::getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	(void)dir;
	return (level.getData(x, y, z) & 8) != 0;
}

bool DetectorRailTile::getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	return (level.getData(x, y, z) & 8) != 0 && dir == 1;
}

void DetectorRailTile::entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	(void)entity;
	if (!level.isOnline)
	{
		int_t data = level.getData(x, y, z);
		if ((data & 8) == 0)
			updateMinecartState(level, x, y, z, data);
	}
}

void DetectorRailTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;
	if (!level.isOnline)
	{
		int_t data = level.getData(x, y, z);
		if ((data & 8) != 0)
			updateMinecartState(level, x, y, z, data);
	}
}

void DetectorRailTile::updateMinecartState(Level &level, int_t x, int_t y, int_t z, int_t data)
{
	bool powered = (data & 8) != 0;
	bool hasMinecart = false;
	float inset = 2.0f / 16.0f;
	AABB *aabb = AABB::newTemp(x + inset, y, z + inset, x + 1 - inset, y + 0.25, z + 1 - inset);
	const auto &entities = level.getEntities(nullptr, *aabb);
	for (const auto &candidate : entities)
	{
		if (dynamic_cast<EntityMinecart *>(candidate.get()) != nullptr)
		{
			hasMinecart = true;
			break;
		}
	}

	if (hasMinecart && !powered)
	{
		setRailData(level, x, y, z, data | 8);
		level.notifyBlocksOfNeighborChange(x, y, z, id);
		level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
	}
	else if (!hasMinecart && powered)
	{
		setRailData(level, x, y, z, data & 7);
		level.notifyBlocksOfNeighborChange(x, y, z, id);
		level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
	}

	if (hasMinecart)
		level.scheduleBlockUpdate(x, y, z, id, getTickDelay());
}
