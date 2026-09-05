#pragma once

#include "world/level/tile/FlowerTile.h"

class DeadBushTile : public FlowerTile
{
public:
	DeadBushTile(int_t id, int_t tex);

	int_t getResource(int_t data, Random &random) override;
	void updateDefaultShape() override;

protected:
	bool canSurviveOn(int_t belowTile) const override;
};
