#include "world/item/RecordItem.h"

#include "world/item/ItemInstance.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/JukeboxTile.h"

RecordItem::RecordItem(int_t baseId, const jstring &recordName) : Item(baseId), recordName(recordName)
{
	setMaxStackSize(1);
}

bool RecordItem::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	(void)face;
	if (level.getTile(x, y, z) != 84 || level.getData(x, y, z) != 0)
		return false;
	auto *jukebox = dynamic_cast<JukeboxTile *>(Tile::tiles[84]);
	if (jukebox == nullptr)
		return false;
	jukebox->insertRecord(level, x, y, z, getShiftedIndex());
	level.playRecord(recordName, x, y, z);
	stack.stackSize--;
	return true;
}