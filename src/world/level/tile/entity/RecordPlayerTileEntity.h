#pragma once

#include "world/level/tile/entity/TileEntity.h"

class RecordPlayerTileEntity : public TileEntity
{
public:
	jstring getEncodeId() const override { return u"RecordPlayer"; }

	int_t record = 0;

	void load(CompoundTag &tag) override;
	void save(CompoundTag &tag) override;
};