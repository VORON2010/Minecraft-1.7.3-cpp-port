#pragma once

#include "world/level/tile/FlowerTile.h"

class MushroomTile : public FlowerTile
{
public:
	MushroomTile(int_t id, int_t tex);

	void updateDefaultShape() override;

protected:
	bool canSurviveOn(int_t belowTile) const override;
	bool canStay(Level &level, int_t x, int_t y, int_t z) override;
};
