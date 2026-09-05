#pragma once

#include "world/level/tile/Tile.h"

class RailLogic;

class RailTile : public Tile
{
private:
	bool poweredRail = false;

	void updateRail(Level &level, int_t x, int_t y, int_t z, bool forceUpdate);
	bool updatePoweredState(Level &level, int_t x, int_t y, int_t z, int_t data);
	bool isSupported(Level &level, int_t x, int_t y, int_t z, int_t shapeData) const;

protected:
	friend class RailLogic;

	void setRailData(Level &level, int_t x, int_t y, int_t z, int_t data) const;
	bool isPoweredRail() const { return poweredRail; }

public:
	RailTile(int_t id, int_t tex, bool poweredRail);

	static bool isRail(LevelSource &level, int_t x, int_t y, int_t z);
	static bool isRail(int_t tileId);

	bool isCubeShaped() override { return false; }
	bool isSolidRender() override { return false; }
	Shape getRenderShape() override { return SHAPE_RAIL; }
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override { return nullptr; }
	AABB *getTileAABB(Level &level, int_t x, int_t y, int_t z) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;
	int_t getTexture(Facing face, int_t data) override;
	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	int_t getMobilityFlag() const override { return 0; }
};
