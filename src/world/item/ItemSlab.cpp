#include "world/item/ItemSlab.h"

#include "world/item/ItemInstance.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/SlabTile.h"
#include "world/level/tile/Tile.h"
#include "world/phys/AABB.h"

namespace
{
	bool isReplaceable(Level &level, int_t x, int_t y, int_t z)
	{
		int_t existingId = level.getTile(x, y, z);
		return existingId == 0 || existingId == 78;
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

	bool canPlace(Level &level, Tile &tile, int_t x, int_t y, int_t z)
	{
		if (!isReplaceable(level, x, y, z))
			return false;
		AABB *placementBox = tile.getAABB(level, x, y, z);
		return placementBox == nullptr || level.isUnobstructed(*placementBox);
	}
}

ItemSlab::ItemSlab(int_t baseId) : Item(baseId)
{
	setHasSubtypes(true);
}

int_t ItemSlab::getIcon(const ItemInstance &stack) const
{
	return Tile::slabSingle.getTexture(Facing::NORTH, stack.itemDamage);
}

jstring ItemSlab::getDescriptionId(const ItemInstance &stack) const
{
	
	static const jstring names[] = {u"stone", u"sand", u"wood", u"cobble"};
	return Tile::slabSingle.descriptionId + u"." + names[stack.itemDamage & 3];
}

bool ItemSlab::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	if (stack.isEmpty())
		return false;

	int_t variant = stack.itemDamage & 3;
	int_t placeX = x;
	int_t placeY = y;
	int_t placeZ = z;
	if (level.getTile(x, y, z) != 78)
		moveToPlacementTarget(placeX, placeY, placeZ, face);

	if (placeY > 0 && isReplaceable(level, placeX, placeY, placeZ) &&
		level.getTile(placeX, placeY - 1, placeZ) == Tile::slabSingle.id &&
		(level.getData(placeX, placeY - 1, placeZ) & 3) == variant)
	{
		AABB *placementBox = Tile::slabDouble.getAABB(level, placeX, placeY - 1, placeZ);
		if (placementBox != nullptr && !level.isUnobstructed(*placementBox))
			return true;
		if (!level.setTileAndData(placeX, placeY - 1, placeZ, Tile::slabDouble.id, variant))
			return true;
		if (Tile::slabDouble.soundType != nullptr)
		{
			StepSound *ss = Tile::slabDouble.soundType;
			level.playSoundEffect((double)placeX + 0.5, (double)(placeY - 1) + 0.5, (double)placeZ + 0.5,
				ss->getStepResourcePath(), (ss->getVolume() + 1.0f) / 2.0f, ss->getPitch() * 0.8f);
		}
		stack.stackSize--;
		return true;
	}

	if (!canPlace(level, Tile::slabSingle, placeX, placeY, placeZ))
		return true;
	if (!level.setTileAndData(placeX, placeY, placeZ, Tile::slabSingle.id, variant))
		return true;

	int_t placedId = level.getTile(placeX, placeY, placeZ);
	bool mergedDown = placeY > 0 && level.getTile(placeX, placeY - 1, placeZ) == Tile::slabDouble.id && (level.getData(placeX, placeY - 1, placeZ) & 3) == variant;
	if (placedId != Tile::slabSingle.id && !mergedDown)
		return true;

	if (Tile::slabSingle.soundType != nullptr)
	{
		StepSound *ss = Tile::slabSingle.soundType;
		level.playSoundEffect((double)placeX + 0.5, (double)placeY + 0.5, (double)placeZ + 0.5,
			ss->getStepResourcePath(), (ss->getVolume() + 1.0f) / 2.0f, ss->getPitch() * 0.8f);
	}
	stack.stackSize--;
	return true;
}
