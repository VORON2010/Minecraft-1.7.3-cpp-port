#include "world/level/tile/FireTile.h"

#include "world/level/tile/PortalTile.h"

#include "world/entity/Entity.h"
#include "world/level/Level.h"
#include "world/level/material/GasMaterial.h"
#include "world/level/material/Material.h"
#include "world/level/material/DecorationMaterial.h"
#include "world/level/tile/Tile.h"

namespace
{
	bool isFireTileId(int_t tileId)
	{
		if (tileId <= 0 || tileId >= static_cast<int_t>(Tile::tiles.size()))
			return false;
		Tile *tile = Tile::tiles[tileId];
		return dynamic_cast<FireTile *>(tile) != nullptr;
	}
}

PortalTile::PortalTile(int_t id, int_t tex) : TransparentTile(id, tex, Material::portal, false)
{
}

AABB *PortalTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	return nullptr;
}

void PortalTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	bool alongX = level.getTile(x - 1, y, z) == id || level.getTile(x + 1, y, z) == id;
	if (alongX)
	{
		float halfWidth = 0.5f;
		float halfDepth = 2.0f / 16.0f;
		setShape(0.5f - halfWidth, 0.0f, 0.5f - halfDepth, 0.5f + halfWidth, 1.0f, 0.5f + halfDepth);
		return;
	}

	float halfWidth = 2.0f / 16.0f;
	float halfDepth = 0.5f;
	setShape(0.5f - halfWidth, 0.0f, 0.5f - halfDepth, 0.5f + halfWidth, 1.0f, 0.5f + halfDepth);
}

bool PortalTile::isCubeShaped()
{
	return false;
}

bool PortalTile::shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	(void)face;
	
	return level.getTile(x, y, z) != id;
}

int_t PortalTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

int_t PortalTile::getRenderLayer()
{
	return 1;
}

void PortalTile::entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	if (entity.riding != nullptr || entity.rider != nullptr)
		return;
	entity.handleInsidePortal();
}

void PortalTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (random.nextInt(100) == 0)
	{
		level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5,
			u"portal.portal", 1.0f, random.nextFloat() * 0.4f + 0.8f);
	}

	for (int_t i = 0; i < 4; ++i)
	{
		double particleX = static_cast<double>(x) + random.nextFloat();
		double particleY = static_cast<double>(y) + random.nextFloat();
		double particleZ = static_cast<double>(z) + random.nextFloat();
		double motionX = (random.nextFloat() - 0.5) * 0.5;
		double motionY = (random.nextFloat() - 0.5) * 0.5;
		double motionZ = (random.nextFloat() - 0.5) * 0.5;
		int_t dir = random.nextInt(2) * 2 - 1;

		if (level.getTile(x - 1, y, z) != id && level.getTile(x + 1, y, z) != id)
		{
			particleX = static_cast<double>(x) + 0.5 + 0.25 * dir;
			motionX = random.nextFloat() * 2.0f * dir;
		}
		else
		{
			particleZ = static_cast<double>(z) + 0.5 + 0.25 * dir;
			motionZ = random.nextFloat() * 2.0f * dir;
		}

		level.addParticle(u"portal", particleX, particleY, particleZ, motionX, motionY, motionZ);
	}
}

void PortalTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	int_t stepX = 0;
	int_t stepZ = 1;
	if (level.getTile(x - 1, y, z) == id || level.getTile(x + 1, y, z) == id)
	{
		stepX = 1;
		stepZ = 0;
	}

	int_t bottomY = y;
	while (level.getTile(x, bottomY - 1, z) == id)
		--bottomY;

	if (level.getTile(x, bottomY - 1, z) != Tile::obsidian.id)
	{
		level.setTile(x, y, z, 0);
		return;
	}

	int_t portalHeight = 1;
	while (portalHeight < 4 && level.getTile(x, bottomY + portalHeight, z) == id)
		++portalHeight;

	if (portalHeight != 3 || level.getTile(x, bottomY + portalHeight, z) != Tile::obsidian.id)
	{
		level.setTile(x, y, z, 0);
		return;
	}

	bool hasXNeighbor = level.getTile(x - 1, y, z) == id || level.getTile(x + 1, y, z) == id;
	bool hasZNeighbor = level.getTile(x, y, z - 1) == id || level.getTile(x, y, z + 1) == id;
	if (hasXNeighbor && hasZNeighbor)
	{
		level.setTile(x, y, z, 0);
		return;
	}

	bool validLowerFrame = level.getTile(x + stepX, y, z + stepZ) == Tile::obsidian.id && level.getTile(x - stepX, y, z - stepZ) == id;
	bool validUpperFrame = level.getTile(x - stepX, y, z - stepZ) == Tile::obsidian.id && level.getTile(x + stepX, y, z + stepZ) == id;
	if (!validLowerFrame && !validUpperFrame)
		level.setTile(x, y, z, 0);
}

bool PortalTile::trySpawnPortal(Level &level, int_t x, int_t y, int_t z)
{
	int_t axisX = 0;
	int_t axisZ = 0;
	if (level.getTile(x - 1, y, z) == Tile::obsidian.id || level.getTile(x + 1, y, z) == Tile::obsidian.id)
		axisX = 1;
	if (level.getTile(x, y, z - 1) == Tile::obsidian.id || level.getTile(x, y, z + 1) == Tile::obsidian.id)
		axisZ = 1;
	if (axisX == axisZ)
		return false;

	if (level.getTile(x - axisX, y, z - axisZ) == 0)
	{
		x -= axisX;
		z -= axisZ;
	}

	for (int_t frameOffset = -1; frameOffset <= 2; ++frameOffset)
	{
		for (int_t yOffset = -1; yOffset <= 3; ++yOffset)
		{
			bool isFrame = frameOffset == -1 || frameOffset == 2 || yOffset == -1 || yOffset == 3;
			int_t tileId = level.getTile(x + axisX * frameOffset, y + yOffset, z + axisZ * frameOffset);
			if (isFrame)
			{
				if (tileId != Tile::obsidian.id)
					return false;
				continue;
			}

			if (tileId != 0 && !isFireTileId(tileId))
				return false;
		}
	}

	bool oldNoNeighborUpdate = level.noNeighborUpdate;
	level.noNeighborUpdate = true;
	for (int_t frameOffset = 0; frameOffset < 2; ++frameOffset)
	{
		for (int_t yOffset = 0; yOffset < 3; ++yOffset)
			level.setTile(x + axisX * frameOffset, y + yOffset, z + axisZ * frameOffset, id);
	}
	level.noNeighborUpdate = oldNoNeighborUpdate;
	return true;
}
