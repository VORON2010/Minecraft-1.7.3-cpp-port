#include "world/level/material/DecorationMaterial.h"

DecorationMaterial::DecorationMaterial(bool solid, bool blocksLight, bool blocksMotion, bool letsWaterThrough) :
	solidFlag(solid),
	blocksLightFlag(blocksLight),
	blocksMotionFlag(blocksMotion),
	letsWaterThroughFlag(letsWaterThrough)
{
}

bool DecorationMaterial::letsWaterThrough() const
{
	return letsWaterThroughFlag;
}

bool DecorationMaterial::isSolid() const
{
	return solidFlag;
}

bool DecorationMaterial::blocksLight() const
{
	return blocksLightFlag;
}

bool DecorationMaterial::blocksMotion() const
{
	return blocksMotionFlag;
}
