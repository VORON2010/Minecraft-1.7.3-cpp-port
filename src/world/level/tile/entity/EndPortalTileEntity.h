#pragma once

#include "world/level/tile/entity/TileEntity.h"

class EndPortalTileEntity : public TileEntity
{
public:
	jstring getEncodeId() const override { return u"Airportal"; }
};
