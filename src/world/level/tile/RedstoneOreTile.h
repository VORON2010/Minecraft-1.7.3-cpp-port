#pragma once

#include "world/level/tile/Tile.h"

class RedstoneOreTile : public Tile
{
public:
	bool glowing;
	RedstoneOreTile(int_t id, int_t tex, bool glowing);
	int_t getTickDelay() override;
	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
	void attack(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void stepOn(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;

private:
	void interact(Level &level, int_t x, int_t y, int_t z);
	void poofParticles(Level &level, int_t x, int_t y, int_t z);
};