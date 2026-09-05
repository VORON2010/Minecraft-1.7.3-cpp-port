#include "world/level/tile/entity/PistonTileEntity.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/PistonMovingTile.h"
#include "world/level/tile/PistonTextures.h"
#include "world/entity/Entity.h"
#include "world/phys/AABB.h"
#include "nbt/CompoundTag.h"

PistonTileEntity::PistonTileEntity(int_t blockId, int_t blockData, int_t facing, bool ext, bool head)
	: storedBlockID(blockId), storedData(blockData), direction(facing), extending(ext), renderHead(head)
{
}

float PistonTileEntity::getProgress(float partialTick) const
{
	if (partialTick > 1.0f) partialTick = 1.0f;
	return lastProgress + (progress - lastProgress) * partialTick;
}

float PistonTileEntity::getOffsetX(float partialTick) const
{
	float p = getProgress(partialTick);
	return extending ? (p - 1.0f) * PistonTextures::offsetX[direction] : (1.0f - p) * PistonTextures::offsetX[direction];
}

float PistonTileEntity::getOffsetY(float partialTick) const
{
	float p = getProgress(partialTick);
	return extending ? (p - 1.0f) * PistonTextures::offsetY[direction] : (1.0f - p) * PistonTextures::offsetY[direction];
}

float PistonTileEntity::getOffsetZ(float partialTick) const
{
	float p = getProgress(partialTick);
	return extending ? (p - 1.0f) * PistonTextures::offsetZ[direction] : (1.0f - p) * PistonTextures::offsetZ[direction];
}

void PistonTileEntity::pushEntities(float currentProgress, float delta)
{
	float pushProgress = currentProgress;
	if (!extending)
		pushProgress -= 1.0f;
	else
		pushProgress = 1.0f - pushProgress;

	AABB *pushBox = Tile::pistonMoving.getPushedAABB(*level, x, y, z, storedBlockID, pushProgress, direction);
	if (pushBox == nullptr)
		return;

	const auto &entities = level->getEntities(nullptr, *pushBox);
	if (entities.empty())
		return;

	
	
	std::vector<std::shared_ptr<Entity>> pushed(entities.begin(), entities.end());
	for (const auto &entity : pushed)
	{
		entity->move(
			delta * PistonTextures::offsetX[direction],
			delta * PistonTextures::offsetY[direction],
			delta * PistonTextures::offsetZ[direction]
		);
	}
}

void PistonTileEntity::finishMovement()
{
	if (lastProgress < 1.0f)
	{
		lastProgress = progress = 1.0f;
		level->removeTileEntity(x, y, z);
		setChanged();
		if (level->getTile(x, y, z) == Tile::pistonMoving.id)
		{
			level->setTileAndData(x, y, z, storedBlockID, storedData);
		}
	}
}

void PistonTileEntity::tick()
{
	lastProgress = progress;
	if (lastProgress >= 1.0f)
	{
		pushEntities(1.0f, 0.25f);
		level->removeTileEntity(x, y, z);
		setChanged();
		if (level->getTile(x, y, z) == Tile::pistonMoving.id)
		{
			level->setTileAndData(x, y, z, storedBlockID, storedData);
		}
	}
	else
	{
		progress += 0.5f;
		if (progress >= 1.0f)
			progress = 1.0f;

		if (extending)
		{
			pushEntities(progress, progress - lastProgress + 1.0f / 16.0f);
		}
	}
}

void PistonTileEntity::load(CompoundTag &tag)
{
	TileEntity::load(tag);
	storedBlockID = tag.getInt(u"blockId");
	storedData = tag.getInt(u"blockData");
	direction = tag.getInt(u"facing");
	lastProgress = progress = tag.getFloat(u"progress");
	extending = tag.getBoolean(u"extending");
}

void PistonTileEntity::save(CompoundTag &tag)
{
	TileEntity::save(tag);
	tag.putInt(u"blockId", storedBlockID);
	tag.putInt(u"blockData", storedData);
	tag.putInt(u"facing", direction);
	tag.putFloat(u"progress", lastProgress);
	tag.putBoolean(u"extending", extending);
}
