#pragma once

#include "world/level/tile/Tile.h"

class DoorTile : public Tile
{
private:
	bool iron = false;

	int_t getState(int_t data) const;
	void setDoorRotation(int_t state);
	void removePair(Level &level, int_t x, int_t y, int_t z, bool drop, int_t data);

public:
	DoorTile(int_t id, int_t tex, const Material &material, bool iron);

	bool isCubeShaped() override;
	bool isSolidRender() override;
	Shape getRenderShape() override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	AABB *getTileAABB(Level &level, int_t x, int_t y, int_t z) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;
	int_t getTexture(Facing face, int_t data) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	int_t getResource(int_t data, Random &random) override;
	int_t getMobilityFlag() const override { return 1; }
};