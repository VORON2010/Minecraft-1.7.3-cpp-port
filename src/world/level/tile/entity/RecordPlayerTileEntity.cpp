#include "world/level/tile/entity/RecordPlayerTileEntity.h"

void RecordPlayerTileEntity::load(CompoundTag &tag)
{
	TileEntity::load(tag);
	record = tag.getInt(u"Record");
}

void RecordPlayerTileEntity::save(CompoundTag &tag)
{
	TileEntity::save(tag);
	if (record > 0)
		tag.putInt(u"Record", record);
}