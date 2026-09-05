#pragma once

#include "world/level/tile/Tile.h"
#include "world/level/TilePos.h"
#include <unordered_set>
#include <vector>

class RedStoneDustTile : public Tile
{
public:
	RedStoneDustTile(int_t id, int_t tex);

	bool isCubeShaped() override { return false; }
	bool isSolidRender() override { return false; }
	Shape getRenderShape() override { return SHAPE_RED_DUST; }
	bool mayPick(int_t data, bool canPickLiquid) override { return true; }
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override { return nullptr; }
	int_t getTexture(Facing face, int_t data) override;

	bool getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool isSignalSource() override { return wiresProvidePower; }
	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;

	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;

	int_t getResource(int_t data, Random &random) override;
	int_t getColor(LevelSource &level, int_t x, int_t y, int_t z) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;

	static bool isPowerProviderOrWire(LevelSource &level, int_t x, int_t y, int_t z, int_t dir);

private:
	bool wiresProvidePower = true;
	
	
	std::vector<TilePos> deferredNotifications;
	static int_t WIRE_ID;

	void updateAndPropagateCurrentStrength(Level &level, int_t x, int_t y, int_t z);
	void propagateCurrentStrength(Level &level, int_t x, int_t y, int_t z, int_t fromX, int_t fromY, int_t fromZ);
	void notifyWireNeighborsOfNeighborChange(Level &level, int_t x, int_t y, int_t z);
	int_t getMaxCurrentStrength(Level &level, int_t x, int_t y, int_t z, int_t currentMax);
};