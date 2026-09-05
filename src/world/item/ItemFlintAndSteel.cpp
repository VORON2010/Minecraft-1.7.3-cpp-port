#include "world/item/ItemFlintAndSteel.h"

#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/Tile.h"

namespace
{
	void offsetPlacementTarget(int_t &x, int_t &y, int_t &z, Facing face)
	{
		switch (face)
		{
		case Facing::DOWN: --y; break;
		case Facing::UP: ++y; break;
		case Facing::NORTH: --z; break;
		case Facing::SOUTH: ++z; break;
		case Facing::WEST: --x; break;
		case Facing::EAST: ++x; break;
		default: break;
		}
	}

	FireTile *findFireTile()
	{
		for (Tile *tile : Tile::tiles)
		{
			auto *fire = dynamic_cast<FireTile *>(tile);
			if (fire != nullptr)
				return fire;
		}
		return nullptr;
	}
}

ItemFlintAndSteel::ItemFlintAndSteel(int_t baseId) : Item(baseId)
{
	setMaxStackSize(1);
	setMaxDamage(64);
	setIconIndex(5);
	setDescriptionId(u"item.flintAndSteel");
}

bool ItemFlintAndSteel::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	offsetPlacementTarget(x, y, z, face);

	FireTile *fire = findFireTile();
	if (fire != nullptr && level.getTile(x, y, z) == 0)
	{
		level.playSoundEffect((double)x + 0.5, (double)y + 0.5, (double)z + 0.5,
			u"fire.ignite", 1.0f, itemRandom.nextFloat() * 0.4f + 0.8f);
		level.setTile(x, y, z, fire->id);
	}

	stack.damageItem(1, player);
	return true;
}
