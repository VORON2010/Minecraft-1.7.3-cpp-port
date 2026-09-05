#include "world/level/tile/StairTile.h"

#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"

StairTile::StairTile(int_t id, Tile &modelTile) : Tile(id, modelTile.tex, modelTile.material), modelTile(modelTile)
{
	updateCachedProperties();
}

void StairTile::setPieceShape(Tile &tile, int_t data, int_t piece)
{
	data &= 3;
	if (data == 0)
	{
		if (piece == 0) tile.setShape(0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f);
		else tile.setShape(0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else if (data == 1)
	{
		if (piece == 0) tile.setShape(0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f);
		else tile.setShape(0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	}
	else if (data == 2)
	{
		if (piece == 0) tile.setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f);
		else tile.setShape(0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		if (piece == 0) tile.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
		else tile.setShape(0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f);
	}
}

bool StairTile::isCubeShaped()
{
	return false;
}

bool StairTile::isSolidRender()
{
	return false;
}

Tile::Shape StairTile::getRenderShape()
{
	return SHAPE_STAIRS;
}

void StairTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	int_t data = level.getData(x, y, z);
	for (int_t piece = 0; piece < 2 && data >= 0 && data <= 3; ++piece)
	{
		setPieceShape(*this, data, piece);
		Tile::addAABBs(level, x, y, z, bb, aabbList);
	}
	updateDefaultShape();
}

int_t StairTile::getTexture(Facing face, int_t data)
{
	return modelTile.getTexture(face, data);
}

int_t StairTile::getResourceCount(Random &random)
{
	return modelTile.getResourceCount(random);
}

int_t StairTile::getResource(int_t data, Random &random)
{
	return modelTile.getResource(data, random);
}

int_t StairTile::getSpawnResourcesAuxValue(int_t data)
{
	return modelTile.getSpawnResourcesAuxValue(data);
}

void StairTile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t rotation = Mth::floor(static_cast<double>(player.yRot) * 4.0 / 360.0 + 0.5) & 3;
	if (rotation == 0)
		level.setData(x, y, z, 2);
	else if (rotation == 1)
		level.setData(x, y, z, 1);
	else if (rotation == 2)
		level.setData(x, y, z, 3);
	else if (rotation == 3)
		level.setData(x, y, z, 0);
}

void StairTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}