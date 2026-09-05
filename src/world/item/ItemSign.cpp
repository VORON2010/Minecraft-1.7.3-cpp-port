#include "world/item/ItemSign.h"

#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/SignTile.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "client/player/LocalPlayer.h"
#include "util/Mth.h"

ItemSign::ItemSign(int_t id) : Item(id)
{
	setMaxStackSize(1);
}

bool ItemSign::useOn(ItemInstance &instance, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	if (face == Facing::DOWN)
		return false;
	if (!level.getMaterial(x, y, z).isSolid())
		return false;

	if (face == Facing::UP)
		y++;
	if (face == Facing::NORTH)
		z--;
	if (face == Facing::SOUTH)
		z++;
	if (face == Facing::WEST)
		x--;
	if (face == Facing::EAST)
		x++;

	int_t existing = level.getTile(x, y, z);
	if (existing != 0 && !Tile::tiles[existing]->material.isLiquid())
		return false;

	int_t tileId = face == Facing::UP ? Tile::signPost.id : Tile::signWall.id;
	int_t data = face == Facing::UP ? (Mth::floor((player.yRot + 180.0f) * 16.0f / 360.0f + 0.5) & 15) : static_cast<int_t>(face);
	if (!level.setTileAndData(x, y, z, tileId, data))
		return false;
	if (level.getTile(x, y, z) != tileId)
		return false;

	instance.stackSize--;
	auto tileEntity = std::dynamic_pointer_cast<SignTileEntity>(level.getTileEntity(x, y, z));
	if (tileEntity)
	{
		LocalPlayer *localPlayer = dynamic_cast<LocalPlayer *>(&player);
		if (localPlayer)
			localPlayer->openTextEdit(tileEntity);
	}
	return true;
}