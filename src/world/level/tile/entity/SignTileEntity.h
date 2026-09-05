#pragma once

#include <array>

#include "world/level/tile/entity/TileEntity.h"

#include "java/String.h"

class SignTileEntity : public TileEntity
{
public:
	std::array<jstring, 4> signText = {{u"", u"", u"", u""}};
	int_t lineBeingEdited = -1;

	jstring getEncodeId() const override { return u"Sign"; }
	void load(CompoundTag &tag) override;
	void save(CompoundTag &tag) override;
};