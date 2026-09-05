#include "world/item/ItemBed.h"

#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/BedTile.h"
#include "world/level/tile/Tile.h"

ItemBed::ItemBed(int_t baseId) : Item(baseId)
{
	setMaxStackSize(1);
}

bool ItemBed::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	if (face != Facing::UP)
		return false;

	y++;
	int_t dir = Mth::floor(static_cast<double>(player.yRot) * 4.0 / 360.0 + 0.5) & 3;
	int_t dx = 0, dz = 0;
	if (dir == 0) dz = 1;
	if (dir == 1) dx = -1;
	if (dir == 2) dz = -1;
	if (dir == 3) dx = 1;

	if (level.isEmptyTile(x, y, z) && level.isEmptyTile(x + dx, y, z + dz)
		&& level.isSolidTile(x, y - 1, z) && level.isSolidTile(x + dx, y - 1, z + dz))
	{
		level.setTileAndData(x, y, z, Tile::bed.id, dir);
		level.setTileAndData(x + dx, y, z + dz, Tile::bed.id, dir | 8);
		stack.stackSize--;
		return true;
	}
	return false;
}
