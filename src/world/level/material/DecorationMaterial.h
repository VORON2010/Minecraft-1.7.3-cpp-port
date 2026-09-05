#pragma once

#include "world/level/material/Material.h"

class DecorationMaterial : public Material
{
private:
	bool solidFlag = false;
	bool blocksLightFlag = false;
	bool blocksMotionFlag = false;
	bool letsWaterThroughFlag = true;

public:
	DecorationMaterial(bool solid, bool blocksLight, bool blocksMotion, bool letsWaterThrough);

	bool letsWaterThrough() const override;
	bool isSolid() const override;
	bool blocksLight() const override;
	bool blocksMotion() const override;
};
