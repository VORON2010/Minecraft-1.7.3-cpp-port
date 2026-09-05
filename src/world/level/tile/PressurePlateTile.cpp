#include "world/level/tile/PressurePlateTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/entity/Mob.h"
#include "world/entity/player/Player.h"
#include "world/phys/AABB.h"
#include "java/Random.h"



PressurePlateTile::PressurePlateTile(int_t id, int_t tex, Sensitivity sensitivity, const Material &material)
	: Tile(id, tex, material), sensitivity(sensitivity)
{
	setTicking(true);
	setShape(1.0f / 16.0f, 0.0f, 1.0f / 16.0f, 15.0f / 16.0f, 0.03125f, 15.0f / 16.0f);
	updateCachedProperties();
}

bool PressurePlateTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	
	return level.isBlockNormalCube(x, y - 1, z);
}

void PressurePlateTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	
	if (!level.isBlockNormalCube(x, y - 1, z))
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
	}
}

void PressurePlateTile::entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	(void)entity;
	
	if (level.getData(x, y, z) != 1)
		setStateIfMobInteractsWithPlate(level, x, y, z);
}

void PressurePlateTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;
	
	if (level.getData(x, y, z) != 0)
		setStateIfMobInteractsWithPlate(level, x, y, z);
}

void PressurePlateTile::setStateIfMobInteractsWithPlate(Level &level, int_t x, int_t y, int_t z)
{
	
	float x0 = (float)x + 2.0f / 16.0f;
	float y0 = (float)y;
	float z0 = (float)z + 2.0f / 16.0f;
	float x1 = (float)x + 14.0f / 16.0f;
	float y1 = (float)y + 0.25f;
	float z1 = (float)z + 14.0f / 16.0f;

	AABB *plateAABB = AABB::newTemp(x0, y0, z0, x1, y1, z1);
	const auto &entities = level.getEntities(nullptr, *plateAABB);

	bool found = false;
	for (const auto &e : entities)
	{
		if (!e->isAlive())
			continue;

		switch (sensitivity)
		{
		case Sensitivity::EVERYTHING:
			found = true;
			break;
		case Sensitivity::MOBS:
			if (dynamic_cast<Mob *>(e.get()) != nullptr)
				found = true;
			break;
		case Sensitivity::PLAYERS:
			if (dynamic_cast<Player *>(e.get()) != nullptr)
				found = true;
			break;
		}
		if (found)
			break;
	}

	int_t oldData = level.getData(x, y, z);
	int_t newData = found ? 1 : 0;

	if (oldData != newData)
	{
		level.setData(x, y, z, newData);
		level.notifyBlocksOfNeighborChange(x, y, z, id);
		level.notifyBlocksOfNeighborChange(x, y - 1, z, id);

		float pitch = found ? 0.6f : 0.5f;
		level.playSoundEffect((double)x + 0.5, (double)y + 0.1, (double)z + 0.5, u"random.click", 0.3f, pitch);
	}

	if (found)
		level.scheduleBlockUpdate(x, y, z, id, getTickDelay());
}

bool PressurePlateTile::getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	(void)dir;
	return level.getData(x, y, z) > 0;
}

bool PressurePlateTile::getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	return level.getData(x, y, z) > 0 && dir == 1;
}

void PressurePlateTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	if (data > 0)
	{
		level.notifyBlocksOfNeighborChange(x, y, z, id);
		level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
	}
}

void PressurePlateTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	if (data == 1)
	{
		
		setShape(1.0f / 16.0f, 0.0f, 1.0f / 16.0f, 15.0f / 16.0f, 0.03125f, 15.0f / 16.0f);
	}
	else
	{
		
		setShape(1.0f / 16.0f, 0.0f, 1.0f / 16.0f, 15.0f / 16.0f, 1.0f / 16.0f, 15.0f / 16.0f);
	}
}

void PressurePlateTile::updateDefaultShape()
{
	float width = 0.5f;
	float height = 2.0f / 16.0f;
	float depth = 0.5f;
	setShape(0.5f - width, 0.5f - height, 0.5f - depth, 0.5f + width, 0.5f + height, 0.5f + depth);
}