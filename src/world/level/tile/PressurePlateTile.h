#pragma once
#include "world/level/tile/Tile.h"

class PressurePlateTile : public Tile
{
public:
	enum class Sensitivity { EVERYTHING, MOBS, PLAYERS };

	PressurePlateTile(int_t id, int_t tex, Sensitivity sensitivity, const Material &material);

	bool isCubeShaped() override { return false; }
	bool isSolidRender() override { return false; }
	Shape getRenderShape() override { return SHAPE_BLOCK; }
	bool mayPick(int_t data, bool canPickLiquid) override { return true; }
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override { return nullptr; }
	int_t getTickDelay() override { return 20; }
	int_t getMobilityFlag() const override { return 1; }

	bool getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool isSignalSource() override { return true; }

	bool mayPlace(Level &level, int_t x, int_t y, int_t z);
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override {}
	void entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;

private:
	Sensitivity sensitivity;
	void setStateIfMobInteractsWithPlate(Level &level, int_t x, int_t y, int_t z);
};