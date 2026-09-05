#pragma once

#include "world/level/tile/entity/TileEntity.h"

class NoteTileEntity : public TileEntity
{
public:
	jstring getEncodeId() const override { return u"Music"; }

	byte_t note = 0;
	bool previousRedstoneState = false;

	void load(CompoundTag &tag) override;
	void save(CompoundTag &tag) override;
	void changePitch();
	void triggerNote(Level &level, int_t x, int_t y, int_t z);
};