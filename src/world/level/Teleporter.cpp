#include "world/level/Teleporter.h"

#include "world/entity/Entity.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/PortalTile.h"

#include "util/Mth.h"

void Teleporter::teleport(Level &level, Entity &entity)
{
	if (!findPortal(level, entity))
	{
		createPortal(level, entity);
		findPortal(level, entity);
	}
}

bool Teleporter::findPortal(Level &level, Entity &entity)
{
	short_t range = 128;
	double bestDistance = -1.0;
	int_t bestX = 0;
	int_t bestY = 0;
	int_t bestZ = 0;
	int_t entityX = Mth::floor(entity.x);
	int_t entityZ = Mth::floor(entity.z);

	for (int_t x = entityX - range; x <= entityX + range; x++)
	{
		double dx = x + 0.5 - entity.x;

		for (int_t z = entityZ - range; z <= entityZ + range; z++)
		{
			double dz = z + 0.5 - entity.z;

			for (int_t y = 127; y >= 0; y--)
			{
				if (level.getTile(x, y, z) == Tile::portal.id)
				{
					while (level.getTile(x, y - 1, z) == Tile::portal.id)
						y--;

					double dy = y + 0.5 - entity.y;
					double distance = dx * dx + dy * dy + dz * dz;
					if (bestDistance < 0.0 || distance < bestDistance)
					{
						bestDistance = distance;
						bestX = x;
						bestY = y;
						bestZ = z;
					}
				}
			}
		}
	}

	if (bestDistance >= 0.0)
	{
		double targetX = bestX + 0.5;
		double targetY = bestY + 0.5;
		double targetZ = bestZ + 0.5;
		if (level.getTile(bestX - 1, bestY, bestZ) == Tile::portal.id)
			targetX -= 0.5;
		if (level.getTile(bestX + 1, bestY, bestZ) == Tile::portal.id)
			targetX += 0.5;
		if (level.getTile(bestX, bestY, bestZ - 1) == Tile::portal.id)
			targetZ -= 0.5;
		if (level.getTile(bestX, bestY, bestZ + 1) == Tile::portal.id)
			targetZ += 0.5;

		entity.moveTo(targetX, targetY, targetZ, entity.yRot, 0.0f);
		entity.xd = entity.yd = entity.zd = 0.0;
		return true;
	}

	return false;
}

bool Teleporter::createPortal(Level &level, Entity &entity)
{
	int_t range = 16;
	double bestDistance = -1.0;
	int_t entityX = Mth::floor(entity.x);
	int_t entityY = Mth::floor(entity.y);
	int_t entityZ = Mth::floor(entity.z);
	int_t bestX = entityX;
	int_t bestY = entityY;
	int_t bestZ = entityZ;
	int_t bestOrientation = 0;
	int_t orientationOffset = random.nextInt(4);

	for (int_t x = entityX - range; x <= entityX + range; x++)
	{
		double dx = x + 0.5 - entity.x;

		for (int_t z = entityZ - range; z <= entityZ + range; z++)
		{
			double dz = z + 0.5 - entity.z;

			for (int_t y = 127; y >= 0; y--)
			{
				if (!level.isEmptyTile(x, y, z))
					continue;

				while (y > 0 && level.isEmptyTile(x, y - 1, z))
					y--;

				for (int_t orientation = orientationOffset; orientation < orientationOffset + 4; orientation++)
				{
					int_t xStep = orientation % 2;
					int_t zStep = 1 - xStep;
					if (orientation % 4 >= 2)
					{
						xStep = -xStep;
						zStep = -zStep;
					}

					bool fits = true;
					for (int_t depth = 0; fits && depth < 3; depth++)
					{
						for (int_t width = 0; fits && width < 4; width++)
						{
							for (int_t height = -1; fits && height < 4; height++)
							{
								int_t checkX = x + (width - 1) * xStep + depth * zStep;
								int_t checkY = y + height;
								int_t checkZ = z + (width - 1) * zStep - depth * xStep;
								if ((height < 0 && !level.getMaterial(checkX, checkY, checkZ).isSolid())
									|| (height >= 0 && !level.isEmptyTile(checkX, checkY, checkZ)))
								{
									fits = false;
								}
							}
						}
					}
					if (!fits)
						continue;

					double dy = y + 0.5 - entity.y;
					double distance = dx * dx + dy * dy + dz * dz;
					if (bestDistance < 0.0 || distance < bestDistance)
					{
						bestDistance = distance;
						bestX = x;
						bestY = y;
						bestZ = z;
						bestOrientation = orientation % 4;
					}
				}
			}
		}
	}

	if (bestDistance < 0.0)
	{
		for (int_t x = entityX - range; x <= entityX + range; x++)
		{
			double dx = x + 0.5 - entity.x;

			for (int_t z = entityZ - range; z <= entityZ + range; z++)
			{
				double dz = z + 0.5 - entity.z;

				for (int_t y = 127; y >= 0; y--)
				{
					if (!level.isEmptyTile(x, y, z))
						continue;

					while (y > 0 && level.isEmptyTile(x, y - 1, z))
						y--;

					for (int_t orientation = orientationOffset; orientation < orientationOffset + 2; orientation++)
					{
						int_t xStep = orientation % 2;
						int_t zStep = 1 - xStep;

						bool fits = true;
						for (int_t width = 0; fits && width < 4; width++)
						{
							for (int_t height = -1; fits && height < 4; height++)
							{
								int_t checkX = x + (width - 1) * xStep;
								int_t checkY = y + height;
								int_t checkZ = z + (width - 1) * zStep;
								if ((height < 0 && !level.getMaterial(checkX, checkY, checkZ).isSolid())
									|| (height >= 0 && !level.isEmptyTile(checkX, checkY, checkZ)))
								{
									fits = false;
								}
							}
						}
						if (!fits)
							continue;

						double dy = y + 0.5 - entity.y;
						double distance = dx * dx + dy * dy + dz * dz;
						if (bestDistance < 0.0 || distance < bestDistance)
						{
							bestDistance = distance;
							bestX = x;
							bestY = y;
							bestZ = z;
							bestOrientation = orientation % 2;
						}
					}
				}
			}
		}
	}

	int_t portalX = bestX;
	int_t portalY = bestY;
	int_t portalZ = bestZ;
	int_t xStep = bestOrientation % 2;
	int_t zStep = 1 - xStep;
	if (bestOrientation % 4 >= 2)
	{
		xStep = -xStep;
		zStep = -zStep;
	}

	if (bestDistance < 0.0)
	{
		if (bestY < 70)
			bestY = 70;
		if (bestY > 118)
			bestY = 118;

		portalY = bestY;

		for (int_t depth = -1; depth <= 1; depth++)
		{
			for (int_t width = 1; width < 3; width++)
			{
				for (int_t height = -1; height < 3; height++)
				{
					int_t blockX = portalX + (width - 1) * xStep + depth * zStep;
					int_t blockY = portalY + height;
					int_t blockZ = portalZ + (width - 1) * zStep - depth * xStep;
					bool frame = height < 0;
					level.setTile(blockX, blockY, blockZ, frame ? Tile::obsidian.id : 0);
				}
			}
		}
	}

	for (int_t pass = 0; pass < 4; pass++)
	{
		level.noNeighborUpdate = true;

		for (int_t width = 0; width < 4; width++)
		{
			for (int_t height = -1; height < 4; height++)
			{
				int_t blockX = portalX + (width - 1) * xStep;
				int_t blockY = portalY + height;
				int_t blockZ = portalZ + (width - 1) * zStep;
				bool frame = width == 0 || width == 3 || height == -1 || height == 3;
				level.setTile(blockX, blockY, blockZ, frame ? Tile::obsidian.id : Tile::portal.id);
			}
		}

		level.noNeighborUpdate = false;

		for (int_t width = 0; width < 4; width++)
		{
			for (int_t height = -1; height < 4; height++)
			{
				int_t blockX = portalX + (width - 1) * xStep;
				int_t blockY = portalY + height;
				int_t blockZ = portalZ + (width - 1) * zStep;
				level.notifyBlocksOfNeighborChange(blockX, blockY, blockZ, level.getTile(blockX, blockY, blockZ));
			}
		}
	}

	return true;
}
