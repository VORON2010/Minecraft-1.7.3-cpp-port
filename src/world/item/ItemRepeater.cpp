#include "world/item/ItemRepeater.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/RepeaterTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/StepSound.h"
#include "world/level/tile/Tile.h"
#include "world/phys/AABB.h"

namespace
{
	bool isPlacementReplaceable(Level &level, int_t x, int_t y, int_t z)
	{
		int_t existingId = level.getTile(x, y, z);
		if (existingId == 0 || existingId == Tile::snow.id)
			return true;
		Tile *existingTile = Tile::tiles[existingId];
		return existingTile != nullptr && existingTile->material.isLiquid();
	}

	void moveToPlacementTarget(int_t &x, int_t &y, int_t &z, Facing face)
	{
		switch (face)
		{
		case Facing::DOWN: y--; break;
		case Facing::UP: y++; break;
		case Facing::NORTH: z--; break;
		case Facing::SOUTH: z++; break;
		case Facing::WEST: x--; break;
		case Facing::EAST: x++; break;
		default: break;
		}
	}

	bool canPlaceSelectedTile(Level &level, Tile &tile, int_t x, int_t y, int_t z)
	{
		if (!isPlacementReplaceable(level, x, y, z))
			return false;
		if (!tile.mayPlace(level, x, y, z))
			return false;
		AABB *placementBox = tile.getAABB(level, x, y, z);
		return placementBox == nullptr || level.isUnobstructed(*placementBox);
	}
}

ItemRepeater::ItemRepeater(int_t baseId) : Item(baseId)
{
}

bool ItemRepeater::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	if (stack.isEmpty())
		return false;

	Tile &placedTile = Tile::repeaterIdle;
	int_t placeX = x;
	int_t placeY = y;
	int_t placeZ = z;
	Facing placementFace = face;
	if (level.getTile(x, y, z) != Tile::snow.id)
		moveToPlacementTarget(placeX, placeY, placeZ, face);
	else
		placementFace = Facing::DOWN;

	if (!canPlaceSelectedTile(level, placedTile, placeX, placeY, placeZ))
		return true;
	if (!level.setTileAndData(placeX, placeY, placeZ, placedTile.id, 0))
		return true;
	if (level.getTile(placeX, placeY, placeZ) != placedTile.id)
		return true;

	placedTile.setPlacedOnFace(level, placeX, placeY, placeZ, placementFace);
	placedTile.setPlacedBy(level, placeX, placeY, placeZ, player);
	if (placedTile.soundType != nullptr)
	{
		StepSound *ss = placedTile.soundType;
		level.playSoundEffect((double)placeX + 0.5, (double)placeY + 0.5, (double)placeZ + 0.5,
			ss->getStepResourcePath(), (ss->getVolume() + 1.0f) / 2.0f, ss->getPitch() * 0.8f);
	}

	stack.stackSize--;
	return true;
}
