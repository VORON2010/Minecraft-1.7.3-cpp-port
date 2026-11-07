#pragma once

#include "world/level/tile/Tile.h"

// 1.0 DragonEggBlock: falls like sand, hops away when clicked or hit
class DragonEggTile : public Tile
{
public:
	DragonEggTile(int_t id, int_t tex);

	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void attack(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	int_t getTickDelay() override;
	bool isSolidRender() override;
	bool isCubeShaped() override;
	Shape getRenderShape() override { return SHAPE_DRAGON_EGG; }

private:
	void checkSlide(Level &level, int_t x, int_t y, int_t z);
	void teleport(Level &level, int_t x, int_t y, int_t z);
};
