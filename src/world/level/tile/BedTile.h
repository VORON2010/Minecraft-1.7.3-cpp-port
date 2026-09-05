#pragma once

#include "world/level/tile/Tile.h"

class BedTile : public Tile
{
public:
	static constexpr int_t headBlockToFootBlockMap[4][2] = {{0, 1}, {-1, 0}, {0, -1}, {1, 0}};

	BedTile(int_t id, int_t tex);

	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	int_t getTexture(Facing face, int_t data) override;
	Shape getRenderShape() override { return SHAPE_BED; }
	bool isSolidRender() override { return false; }
	bool isCubeShaped() override { return false; }
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	int_t getResource(int_t data, Random &random) override;
	using Tile::spawnResources;
	void spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance) override;
	void updateDefaultShape() override;
	int_t getMobilityFlag() const override { return 1; }

	static int_t getDirectionFromMetadata(int_t data) { return data & 3; }
	static bool isBlockFootOfBed(int_t data) { return (data & 8) != 0; }
	static bool isBedOccupied(int_t data) { return (data & 4) != 0; }
	static void setBedOccupied(Level &level, int_t x, int_t y, int_t z, bool occupied);
	
	
	static bool getNearestEmptySpot(Level &level, int_t x, int_t y, int_t z, int_t skip, int_t &outX, int_t &outY, int_t &outZ);
};
