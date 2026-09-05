#pragma once

#include <vector>

#include "world/level/tile/TransparentTile.h"

class LeafTile : public TransparentTile
{
public:
	static constexpr int_t REQUIRED_WOOD_RANGE = 4;

	static constexpr int_t CHECK_DECAY_BIT = 1 << 3;
	static constexpr int_t OAK_LEAF = 0;
	static constexpr int_t SPRUCE_LEAF = 1;
	static constexpr int_t BIRCH_LEAF = 2;
	static constexpr int_t LEAF_TYPE_MASK = 0x3;

private:
	int_t oTex = 0;
	std::vector<int_t> adjacentTreeBlocks;

public:
	LeafTile(int_t id, int_t tex);

	int_t getColor(LevelSource &level, int_t x, int_t y, int_t z) override;
	int_t getItemColor(int_t data) override;

	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;

private:
	void die(Level &level, int_t x, int_t y, int_t z);

public:
	int_t getResourceCount(Random &random) override;
	int_t getResource(int_t data, Random &random) override;
	int_t getSpawnResourcesAuxValue(int_t data) override;

	bool isSolidRender() override;

	int_t getTexture(Facing face, int_t data) override;

	void setFancy(bool fancy);

	void harvestBlock(Level &level, Player &player, int_t x, int_t y, int_t z, int_t data) override;
	void stepOn(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;
	int_t getMobilityFlag() const override { return 1; }
};