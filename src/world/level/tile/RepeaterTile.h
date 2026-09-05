#pragma once

#include "world/level/tile/Tile.h"

class RepeaterTile : public Tile
{
private:
	bool repeaterPowered = false;

	bool hasInputSignal(Level &level, int_t x, int_t y, int_t z, int_t data) const;
	bool canSurvive(Level &level, int_t x, int_t y, int_t z) const;
	static int_t getDelayTicks(int_t data);

public:
	explicit RepeaterTile(int_t id, bool repeaterPowered);

	bool isSolidRender() override { return false; }
	bool isCubeShaped() override { return false; }
	Shape getRenderShape() override { return SHAPE_REPEATER; }

	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;

	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;
	bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;

	bool getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	
	
	bool isSignalSource() override { return false; }
	bool getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	int_t getTexture(Facing face, int_t data) override;
	int_t getResource(int_t data, Random &random) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;

	static int_t getOutputDirection(int_t data);
	static int_t getWireConnectionDirection(int_t data);
	static double getDelayTorchOffset(int_t delaySetting);
};
