#include "world/level/tile/ButtonTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"



ButtonTile::ButtonTile(int_t id, int_t tex) : Tile(id, tex, Material::circuits())
{
	setTicking(true);
	updateCachedProperties();
}


int_t ButtonTile::getOrientation(Level &level, int_t x, int_t y, int_t z)
{
	if (level.isBlockNormalCube(x - 1, y, z)) return 1; 
	if (level.isBlockNormalCube(x + 1, y, z)) return 2; 
	if (level.isBlockNormalCube(x, y, z - 1)) return 3; 
	if (level.isBlockNormalCube(x, y, z + 1)) return 4; 
	return -1;
}

bool ButtonTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	
	return getOrientation(level, x, y, z) != -1;
}

void ButtonTile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	
	int_t orient = -1;
	if (face == Facing::NORTH) orient = 4; 
	else if (face == Facing::SOUTH) orient = 3; 
	else if (face == Facing::WEST) orient = 2; 
	else if (face == Facing::EAST) orient = 1; 

	
	if (orient == -1)
		orient = getOrientation(level, x, y, z);

	if (orient == -1)
	{
		
		spawnResources(level, x, y, z, 0);
		level.setTile(x, y, z, 0);
		return;
	}

	
	int_t data = level.getData(x, y, z);
	level.setData(x, y, z, (data & 8) | orient);
}

void ButtonTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	int_t data = level.getData(x, y, z);
	int_t orient = data & 7;

	
	bool supported = false;
	if (orient == 1) supported = level.isBlockNormalCube(x - 1, y, z);
	else if (orient == 2) supported = level.isBlockNormalCube(x + 1, y, z);
	else if (orient == 3) supported = level.isBlockNormalCube(x, y, z - 1);
	else if (orient == 4) supported = level.isBlockNormalCube(x, y, z + 1);

	if (!supported)
	{
		spawnResources(level, x, y, z, data);
		level.setTile(x, y, z, 0);
	}
}

bool ButtonTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	int_t data = level.getData(x, y, z);

	
	if ((data & 8) != 0)
		return true;

	
	level.setData(x, y, z, data | 8);

	
	level.notifyBlocksOfNeighborChange(x, y, z, id);
	int_t orient = data & 7;
	if (orient == 1) level.notifyBlocksOfNeighborChange(x - 1, y, z, id);
	else if (orient == 2) level.notifyBlocksOfNeighborChange(x + 1, y, z, id);
	else if (orient == 3) level.notifyBlocksOfNeighborChange(x, y, z - 1, id);
	else if (orient == 4) level.notifyBlocksOfNeighborChange(x, y, z + 1, id);

	
	level.scheduleBlockUpdate(x, y, z, id, getTickDelay());

	
	level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, u"random.click", 0.3f, 0.6f);

	return true;
}

void ButtonTile::attack(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	use(level, x, y, z, player);
}

void ButtonTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;
	int_t data = level.getData(x, y, z);

	
	if ((data & 8) == 0)
		return;

	
	level.setData(x, y, z, data & ~8);

	
	level.notifyBlocksOfNeighborChange(x, y, z, id);
	int_t orient = data & 7;
	if (orient == 1) level.notifyBlocksOfNeighborChange(x - 1, y, z, id);
	else if (orient == 2) level.notifyBlocksOfNeighborChange(x + 1, y, z, id);
	else if (orient == 3) level.notifyBlocksOfNeighborChange(x, y, z - 1, id);
	else if (orient == 4) level.notifyBlocksOfNeighborChange(x, y, z + 1, id);

	
	level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, u"random.click", 0.3f, 0.5f);
}

bool ButtonTile::getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	(void)dir;
	return (level.getData(x, y, z) & 8) != 0;
}

bool ButtonTile::getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir)
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

void ButtonTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	if ((data & 8) != 0)
	{
		level.notifyBlocksOfNeighborChange(x, y, z, id);
		int_t orient = data & 7;
		if (orient == 1) level.notifyBlocksOfNeighborChange(x - 1, y, z, id);
		else if (orient == 2) level.notifyBlocksOfNeighborChange(x + 1, y, z, id);
		else if (orient == 3) level.notifyBlocksOfNeighborChange(x, y, z - 1, id);
		else if (orient == 4) level.notifyBlocksOfNeighborChange(x, y, z + 1, id);
	}
}

void ButtonTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	
	int_t data = level.getData(x, y, z);
	int_t orient = data & 7;
	bool powered = (data & 8) != 0;

	float y0 = 6.0f / 16.0f;
	float y1 = 10.0f / 16.0f;
	float halfWidth = 3.0f / 16.0f;
	float depth = powered ? 1.0f / 16.0f : 2.0f / 16.0f;
	if (orient == 1)
		setShape(0.0f, y0, 0.5f - halfWidth, depth, y1, 0.5f + halfWidth);
	else if (orient == 2)
		setShape(1.0f - depth, y0, 0.5f - halfWidth, 1.0f, y1, 0.5f + halfWidth);
	else if (orient == 3)
		setShape(0.5f - halfWidth, y0, 0.0f, 0.5f + halfWidth, y1, depth);
	else if (orient == 4)
		setShape(0.5f - halfWidth, y0, 1.0f - depth, 0.5f + halfWidth, y1, 1.0f);
}

void ButtonTile::updateDefaultShape()
{
	float width = 3.0f / 16.0f;
	float height = 2.0f / 16.0f;
	float depth = 2.0f / 16.0f;
	setShape(0.5f - width, 0.5f - height, 0.5f - depth, 0.5f + width, 0.5f + height, 0.5f + depth);
}