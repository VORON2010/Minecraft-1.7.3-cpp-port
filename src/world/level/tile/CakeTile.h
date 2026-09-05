#pragma once

#include "world/level/tile/Tile.h"

class CakeTile : public Tile
{
public:
	CakeTile(int_t id, int_t tex);

	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;

	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	AABB *getTileAABB(Level &level, int_t x, int_t y, int_t z) override;

	int_t getTexture(Facing face, int_t data) override;
	int_t getTexture(Facing face) override;

	bool isCubeShaped() override;
	bool isSolidRender() override;

	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void attack(Level &level, int_t x, int_t y, int_t z, Player &player) override;

	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	bool canSurvive(Level &level, int_t x, int_t y, int_t z);

	int_t getResourceCount(Random &random) override;
	int_t getResource(int_t data, Random &random) override;

private:
	void eat(Level &level, int_t x, int_t y, int_t z, Player &player);
};
