#pragma once

#include "world/level/tile/FlowerTile.h"

class TallGrassTile : public FlowerTile
{
public:
	TallGrassTile(int_t id, int_t tex);

	int_t getTexture(Facing face, int_t data) override;
	int_t getColor(LevelSource &level, int_t x, int_t y, int_t z) override;
	int_t getItemColor(int_t data) override;
	int_t getResource(int_t data, Random &random) override;
	void updateDefaultShape() override;
};
