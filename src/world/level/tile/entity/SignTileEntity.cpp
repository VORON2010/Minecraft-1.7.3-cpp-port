#include "world/level/tile/entity/SignTileEntity.h"

#include "nbt/CompoundTag.h"

void SignTileEntity::load(CompoundTag &tag)
{
	TileEntity::load(tag);
	for (int_t i = 0; i < 4; i++)
	{
		jstring key = u"Text" + String::toString(i + 1);
		signText[i] = tag.getString(key);
		if (signText[i].length() > 15)
			signText[i] = signText[i].substr(0, 15);
	}
}

void SignTileEntity::save(CompoundTag &tag)
{
	TileEntity::save(tag);
	for (int_t i = 0; i < 4; i++)
	{
		jstring key = u"Text" + String::toString(i + 1);
		tag.putString(key, signText[i]);
	}
}