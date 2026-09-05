#include "world/level/tile/DoorTile.h"

#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

DoorTile::DoorTile(int_t id, int_t tex, const Material &material, bool iron) : Tile(id, tex, material), iron(iron)
{
	updateCachedProperties();
}

int_t DoorTile::getState(int_t data) const
{
	return (data & 4) == 0 ? ((data - 1) & 3) : (data & 3);
}

void DoorTile::setDoorRotation(int_t state)
{
	float thickness = 3.0f / 16.0f;
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, thickness);
	if (state == 1)
		setShape(1.0f - thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	if (state == 2)
		setShape(0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f, 1.0f);
	if (state == 3)
		setShape(0.0f, 0.0f, 0.0f, thickness, 1.0f, 1.0f);
}

void DoorTile::removePair(Level &level, int_t x, int_t y, int_t z, bool drop, int_t data)
{
	bool hasTop = level.getTile(x, y + 1, z) == id;
	level.noNeighborUpdate = true;
	level.setTileNoUpdate(x, y, z, 0);
	if (hasTop)
		level.setTileNoUpdate(x, y + 1, z, 0);
	level.noNeighborUpdate = false;
	level.tileUpdated(x, y, z, 0);
	if (hasTop)
		level.tileUpdated(x, y + 1, z, 0);
	if (drop)
		spawnResources(level, x, y, z, data);
}

bool DoorTile::isCubeShaped()
{
	return false;
}

bool DoorTile::isSolidRender()
{
	return false;
}

Tile::Shape DoorTile::getRenderShape()
{
	return SHAPE_DOOR;
}

AABB *DoorTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}

AABB *DoorTile::getTileAABB(Level &level, int_t x, int_t y, int_t z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}

void DoorTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	setDoorRotation(getState(level.getData(x, y, z)));
}

void DoorTile::updateDefaultShape()
{
	setDoorRotation(0);
}

int_t DoorTile::getTexture(Facing face, int_t data)
{
	if (face == Facing::DOWN || face == Facing::UP)
		return tex;

	int_t state = getState(data);
	bool northSouthFace = face == Facing::NORTH || face == Facing::SOUTH;
	if (((state == 0 || state == 2) ^ northSouthFace))
	{
		return tex;
	}

	int_t variant = state / 2 + ((static_cast<int_t>(face) & 1) ^ state);
	variant += (data & 4) / 4;
	int_t result = tex - (data & 8) * 2;
	if ((variant & 1) != 0)
		result = -result;
	return result;
}

bool DoorTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	if (iron)
		return true;

	int_t data = level.getData(x, y, z);
	if ((data & 8) != 0)
	{
		if (level.getTile(x, y - 1, z) == id)
			return use(level, x, y - 1, z, player);
		return true;
	}

	int_t newData = data ^ 4;
	level.noNeighborUpdate = true;
	level.setDataNoUpdate(x, y, z, newData);
	if (level.getTile(x, y + 1, z) == id)
		level.setDataNoUpdate(x, y + 1, z, newData + 8);
	level.noNeighborUpdate = false;
	level.tileUpdated(x, y, z, id);
	if (level.getTile(x, y + 1, z) == id)
		level.tileUpdated(x, y + 1, z, id);
	level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, ((newData & 4) != 0) ? u"random.door_open" : u"random.door_close", 1.0f, level.random.nextFloat() * 0.1f + 0.9f);
	return true;
}

void DoorTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	int_t data = level.getData(x, y, z);
	if ((data & 8) != 0)
	{
		if (level.getTile(x, y - 1, z) != id)
			level.setTile(x, y, z, 0);
		if (tile > 0 && Tile::tiles[tile] != nullptr && Tile::tiles[tile]->isSignalSource() && level.getTile(x, y - 1, z) == id)
			neighborChanged(level, x, y - 1, z, tile);
		return;
	}

	if (level.getTile(x, y + 1, z) != id)
	{
		removePair(level, x, y, z, true, data);
		return;
	}

	if (!level.isSolidTile(x, y - 1, z))
	{
		removePair(level, x, y, z, true, data);
		return;
	}

	if (tile > 0 && Tile::tiles[tile] != nullptr && Tile::tiles[tile]->isSignalSource())
	{
		bool powered = level.hasNeighborSignal(x, y, z) || level.hasNeighborSignal(x, y + 1, z);
		bool isOpen = (data & 4) != 0;
		if (isOpen != powered)
		{
			int_t newData = data ^ 4;
			level.noNeighborUpdate = true;
			level.setDataNoUpdate(x, y, z, newData);
			if (level.getTile(x, y + 1, z) == id)
				level.setDataNoUpdate(x, y + 1, z, newData + 8);
			level.noNeighborUpdate = false;
			level.tileUpdated(x, y, z, id);
			if (level.getTile(x, y + 1, z) == id)
				level.tileUpdated(x, y + 1, z, id);
			level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, ((newData & 4) != 0) ? u"random.door_open" : u"random.door_close", 1.0f, level.random.nextFloat() * 0.1f + 0.9f);
		}
	}

}

int_t DoorTile::getResource(int_t data, Random &random)
{
	(void)random;
	if ((data & 8) != 0)
		return 0;
	return iron ? Items::doorIron->getShiftedIndex() : Items::doorWood->getShiftedIndex();
}