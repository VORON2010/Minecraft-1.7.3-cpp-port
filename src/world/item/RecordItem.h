#pragma once

#include "world/item/Item.h"

class JukeboxTile;

class RecordItem : public Item
{
private:
	jstring recordName;

public:
	RecordItem(int_t baseId, const jstring &recordName);
	const jstring &getRecordName() const { return recordName; }

	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};
