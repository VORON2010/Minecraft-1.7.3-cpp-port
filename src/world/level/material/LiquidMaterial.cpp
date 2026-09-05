#include "world/level/material/LiquidMaterial.h"

bool LiquidMaterial::isLiquid() const
{
	return true;
}

bool LiquidMaterial::isSolid() const
{
	return false;
}

bool LiquidMaterial::blocksLight() const
{
	return true;
}

bool LiquidMaterial::blocksMotion() const
{
	return false;
}
