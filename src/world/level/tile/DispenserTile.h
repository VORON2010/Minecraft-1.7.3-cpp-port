#pragma once

#include "world/level/tile/Tile.h"

class DispenserTile : public Tile
{
public:
	DispenserTile(int_t id, int_t tex, const Material &material);

	int_t getTexture(Facing face, int_t data) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	int_t getTickDelay() override { return 4; }
private:
	void setDefaultDirection(Level &level, int_t x, int_t y, int_t z) const;
	void dropContents(Level &level, int_t x, int_t y, int_t z) const;
};