#include "world/level/tile/RedStoneDustTile.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/RepeaterTile.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "java/Random.h"

int_t RedStoneDustTile::WIRE_ID = 0;

RedStoneDustTile::RedStoneDustTile(int_t id, int_t tex) : Tile(id, tex, Material::circuits())
{
	WIRE_ID = id;
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f / 16.0f, 1.0f);
	updateCachedProperties();
}

int_t RedStoneDustTile::getTexture(Facing face, int_t data)
{
	(void)face;
	(void)data;
	return tex;
}

int_t RedStoneDustTile::getColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	return 8388608;
}

bool RedStoneDustTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	return level.isBlockNormalCube(x, y - 1, z);
}

bool RedStoneDustTile::isPowerProviderOrWire(LevelSource &level, int_t x, int_t y, int_t z, int_t dir)
{
	int_t tileId = level.getTile(x, y, z);
	if (tileId == WIRE_ID)
		return true;
	if (tileId == 0)
		return false;
	Tile *tile = Tile::tiles[tileId];
	if (tile != nullptr && tile->isSignalSource())
		return true;
	if (dir >= 0 && (tileId == Tile::repeaterIdle.id || tileId == Tile::repeaterActive.id))
		return dir == RepeaterTile::getWireConnectionDirection(level.getData(x, y, z));
	return false;
}

void RedStoneDustTile::updateAndPropagateCurrentStrength(Level &level, int_t x, int_t y, int_t z)
{
	deferredNotifications.clear();
	propagateCurrentStrength(level, x, y, z, x, y, z);

	
	auto notifications = deferredNotifications;
	deferredNotifications.clear();

	for (const TilePos &pos : notifications)
	{
		level.notifyBlocksOfNeighborChange(pos.x, pos.y, pos.z, id);
	}
}

void RedStoneDustTile::propagateCurrentStrength(Level &level, int_t x, int_t y, int_t z, int_t fromX, int_t fromY, int_t fromZ)
{
	int_t oldPower = level.getData(x, y, z);
	int_t newPower = 0;

	wiresProvidePower = false;
	bool indirectlyPowered = level.hasNeighborSignal(x, y, z);
	wiresProvidePower = true;

	if (indirectlyPowered)
	{
		newPower = 15;
	}
	else
	{
		
		static const int_t dx[] = {-1, 1, 0, 0};
		static const int_t dz[] = {0, 0, -1, 1};

		for (int i = 0; i < 4; i++)
		{
			int_t nx = x + dx[i];
			int_t nz = z + dz[i];

			if (nx != fromX || y != fromY || nz != fromZ)
				newPower = getMaxCurrentStrength(level, nx, y, nz, newPower);

			if (level.isBlockNormalCube(nx, y, nz) && !level.isBlockNormalCube(x, y + 1, z))
			{
				
				if (nx != fromX || y + 1 != fromY || nz != fromZ)
					newPower = getMaxCurrentStrength(level, nx, y + 1, nz, newPower);
			}
			else if (!level.isBlockNormalCube(nx, y, nz))
			{
				
				if (nx != fromX || y - 1 != fromY || nz != fromZ)
					newPower = getMaxCurrentStrength(level, nx, y - 1, nz, newPower);
			}
		}

		if (newPower > 0)
			newPower--;
		else
			newPower = 0;
	}

	if (oldPower != newPower)
	{
		level.noNeighborUpdate = true;
		level.setData(x, y, z, newPower);
		level.setTilesDirty(x, y, z, x, y, z);
		level.noNeighborUpdate = false;

		
		
		
		static const int_t rdx[] = {-1, 1, 0, 0};
		static const int_t rdz[] = {0, 0, -1, 1};

		for (int i = 0; i < 4; i++)
		{
			int_t nx = x + rdx[i];
			int_t nz = z + rdz[i];
			int_t ny = y - 1;

			if (level.isBlockNormalCube(nx, y, nz))
				ny += 2; 

			int_t neighborPower = getMaxCurrentStrength(level, nx, y, nz, -1);
			newPower = level.getData(x, y, z);
			if (newPower > 0)
				newPower--;

			if (neighborPower >= 0 && neighborPower != newPower)
				propagateCurrentStrength(level, nx, y, nz, x, y, z);

			neighborPower = getMaxCurrentStrength(level, nx, ny, nz, -1);
			newPower = level.getData(x, y, z);
			if (newPower > 0)
				newPower--;

			if (neighborPower >= 0 && neighborPower != newPower)
				propagateCurrentStrength(level, nx, ny, nz, x, y, z);
		}

		if (oldPower == 0 || newPower == 0)
		{
			deferredNotifications.emplace_back(x, y, z);
			deferredNotifications.emplace_back(x - 1, y, z);
			deferredNotifications.emplace_back(x + 1, y, z);
			deferredNotifications.emplace_back(x, y - 1, z);
			deferredNotifications.emplace_back(x, y + 1, z);
			deferredNotifications.emplace_back(x, y, z - 1);
			deferredNotifications.emplace_back(x, y, z + 1);
		}
	}
}

int_t RedStoneDustTile::getMaxCurrentStrength(Level &level, int_t x, int_t y, int_t z, int_t currentMax)
{
	if (level.getTile(x, y, z) != id)
		return currentMax;
	int_t metadata = level.getData(x, y, z);
	return (metadata > currentMax) ? metadata : currentMax;
}

void RedStoneDustTile::notifyWireNeighborsOfNeighborChange(Level &level, int_t x, int_t y, int_t z)
{
	if (level.getTile(x, y, z) == id)
	{
		level.notifyBlocksOfNeighborChange(x, y, z, id);
		level.notifyBlocksOfNeighborChange(x - 1, y, z, id);
		level.notifyBlocksOfNeighborChange(x + 1, y, z, id);
		level.notifyBlocksOfNeighborChange(x, y, z - 1, id);
		level.notifyBlocksOfNeighborChange(x, y, z + 1, id);
		level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
		level.notifyBlocksOfNeighborChange(x, y + 1, z, id);
	}
}

void RedStoneDustTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onPlace(level, x, y, z);

	updateAndPropagateCurrentStrength(level, x, y, z);
	level.notifyBlocksOfNeighborChange(x, y + 1, z, id);
	level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
	notifyWireNeighborsOfNeighborChange(level, x - 1, y, z);
	notifyWireNeighborsOfNeighborChange(level, x + 1, y, z);
	notifyWireNeighborsOfNeighborChange(level, x, y, z - 1);
	notifyWireNeighborsOfNeighborChange(level, x, y, z + 1);

	if (level.isBlockNormalCube(x - 1, y, z))
		notifyWireNeighborsOfNeighborChange(level, x - 1, y + 1, z);
	else
		notifyWireNeighborsOfNeighborChange(level, x - 1, y - 1, z);

	if (level.isBlockNormalCube(x + 1, y, z))
		notifyWireNeighborsOfNeighborChange(level, x + 1, y + 1, z);
	else
		notifyWireNeighborsOfNeighborChange(level, x + 1, y - 1, z);

	if (level.isBlockNormalCube(x, y, z - 1))
		notifyWireNeighborsOfNeighborChange(level, x, y + 1, z - 1);
	else
		notifyWireNeighborsOfNeighborChange(level, x, y - 1, z - 1);

	if (level.isBlockNormalCube(x, y, z + 1))
		notifyWireNeighborsOfNeighborChange(level, x, y + 1, z + 1);
	else
		notifyWireNeighborsOfNeighborChange(level, x, y - 1, z + 1);
}

void RedStoneDustTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onRemove(level, x, y, z);

	level.notifyBlocksOfNeighborChange(x, y + 1, z, id);
	level.notifyBlocksOfNeighborChange(x, y - 1, z, id);
	updateAndPropagateCurrentStrength(level, x, y, z);
	notifyWireNeighborsOfNeighborChange(level, x - 1, y, z);
	notifyWireNeighborsOfNeighborChange(level, x + 1, y, z);
	notifyWireNeighborsOfNeighborChange(level, x, y, z - 1);
	notifyWireNeighborsOfNeighborChange(level, x, y, z + 1);

	if (level.isBlockNormalCube(x - 1, y, z))
		notifyWireNeighborsOfNeighborChange(level, x - 1, y + 1, z);
	else
		notifyWireNeighborsOfNeighborChange(level, x - 1, y - 1, z);

	if (level.isBlockNormalCube(x + 1, y, z))
		notifyWireNeighborsOfNeighborChange(level, x + 1, y + 1, z);
	else
		notifyWireNeighborsOfNeighborChange(level, x + 1, y - 1, z);

	if (level.isBlockNormalCube(x, y, z - 1))
		notifyWireNeighborsOfNeighborChange(level, x, y + 1, z - 1);
	else
		notifyWireNeighborsOfNeighborChange(level, x, y - 1, z - 1);

	if (level.isBlockNormalCube(x, y, z + 1))
		notifyWireNeighborsOfNeighborChange(level, x, y + 1, z + 1);
	else
		notifyWireNeighborsOfNeighborChange(level, x, y - 1, z + 1);
}

void RedStoneDustTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	if (!mayPlace(level, x, y, z))
	{
		int_t data = level.getData(x, y, z);
		spawnResources(level, x, y, z, data);
		level.setTile(x, y, z, 0);
	}
	else
	{
		updateAndPropagateCurrentStrength(level, x, y, z);
	}
	Tile::neighborChanged(level, x, y, z, tile);
}

bool RedStoneDustTile::getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	if (!wiresProvidePower)
		return false;
	if (level.getData(x, y, z) == 0)
		return false;
	if (dir == 1) 
		return true;

	
	bool connectsWest = isPowerProviderOrWire(level, x - 1, y, z, 1)
		|| (!level.isBlockNormalCube(x - 1, y, z) && isPowerProviderOrWire(level, x - 1, y - 1, z, -1));
	bool connectsEast = isPowerProviderOrWire(level, x + 1, y, z, 3)
		|| (!level.isBlockNormalCube(x + 1, y, z) && isPowerProviderOrWire(level, x + 1, y - 1, z, -1));
	bool connectsNorth = isPowerProviderOrWire(level, x, y, z - 1, 2)
		|| (!level.isBlockNormalCube(x, y, z - 1) && isPowerProviderOrWire(level, x, y - 1, z - 1, -1));
	bool connectsSouth = isPowerProviderOrWire(level, x, y, z + 1, 0)
		|| (!level.isBlockNormalCube(x, y, z + 1) && isPowerProviderOrWire(level, x, y - 1, z + 1, -1));

	
	if (!level.isBlockNormalCube(x, y + 1, z))
	{
		if (level.isBlockNormalCube(x - 1, y, z) && isPowerProviderOrWire(level, x - 1, y + 1, z, -1))
			connectsWest = true;
		if (level.isBlockNormalCube(x + 1, y, z) && isPowerProviderOrWire(level, x + 1, y + 1, z, -1))
			connectsEast = true;
		if (level.isBlockNormalCube(x, y, z - 1) && isPowerProviderOrWire(level, x, y + 1, z - 1, -1))
			connectsNorth = true;
		if (level.isBlockNormalCube(x, y, z + 1) && isPowerProviderOrWire(level, x, y + 1, z + 1, -1))
			connectsSouth = true;
	}

	
	if (!connectsNorth && !connectsEast && !connectsWest && !connectsSouth && dir >= 2 && dir <= 5)
		return true;

	
	if (dir == 2 && connectsNorth && !connectsWest && !connectsEast)
		return true;
	if (dir == 3 && connectsSouth && !connectsWest && !connectsEast)
		return true;
	if (dir == 4 && connectsWest && !connectsNorth && !connectsSouth)
		return true;
	if (dir == 5 && connectsEast && !connectsNorth && !connectsSouth)
		return true;

	return false;
}

bool RedStoneDustTile::getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	if (!wiresProvidePower)
		return false;
	return getSignal(level, x, y, z, dir);
}

int_t RedStoneDustTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Items::redstone->getShiftedIndex();
}

void RedStoneDustTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	int_t power = level.getData(x, y, z);
	if (power > 0)
	{
		double px = (double)x + 0.5 + ((double)random.nextFloat() - 0.5) * 0.2;
		double py = (double)((float)y + 1.0f / 16.0f);
		double pz = (double)z + 0.5 + ((double)random.nextFloat() - 0.5) * 0.2;
		float strength = (float)power / 15.0f;
		float r = strength * 0.6f + 0.4f;
		float g = strength * strength * 0.7f - 0.5f;
		float b = strength * strength * 0.6f - 0.7f;
		if (g < 0.0f) g = 0.0f;
		if (b < 0.0f) b = 0.0f;
		level.addParticle(u"reddust", px, py, pz, (double)r, (double)g, (double)b);
	}
}