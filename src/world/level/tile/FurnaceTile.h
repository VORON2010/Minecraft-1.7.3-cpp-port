#pragma once

#include "world/level/tile/Tile.h"

class FurnaceTile : public Tile
{
private:
	bool lit = false;
	static bool keepContents;

	void setDefaultDirection(Level &level, int_t x, int_t y, int_t z) const;
	void dropContents(Level &level, int_t x, int_t y, int_t z) const;

public:
	FurnaceTile(int_t id, bool lit);

	int_t getTexture(Facing face, int_t data) override;
	int_t getResource(int_t data, Random &random) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;

	static void setLitState(bool lit, Level &level, int_t x, int_t y, int_t z);
};
