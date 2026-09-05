#pragma once

#include "world/level/tile/Tile.h"

class StairTile : public Tile
{
private:
	Tile &modelTile;

public:
	StairTile(int_t id, Tile &modelTile);

	static void setPieceShape(Tile &tile, int_t data, int_t piece);

	bool isCubeShaped() override;
	bool isSolidRender() override;
	Shape getRenderShape() override;
	void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;
	int_t getTexture(Facing face, int_t data) override;
	int_t getResourceCount(Random &random) override;
	int_t getResource(int_t data, Random &random) override;
	int_t getSpawnResourcesAuxValue(int_t data) override;
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void updateDefaultShape() override;
};