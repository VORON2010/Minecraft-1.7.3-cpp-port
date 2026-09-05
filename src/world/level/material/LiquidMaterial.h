#pragma once

#include "world/level/material/Material.h"

class MapColor;

class LiquidMaterial : public Material
{
public:
	LiquidMaterial() { setGroundCover(); setNoPushMobility(); }
	explicit LiquidMaterial(MapColor &color) : LiquidMaterial() { setMapColor(color); }

	bool isLiquid() const override;
	bool isSolid() const override;
	bool blocksLight() const override;
	bool blocksMotion() const override;
};
