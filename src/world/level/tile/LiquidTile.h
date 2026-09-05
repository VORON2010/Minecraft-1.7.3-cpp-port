#pragma once

#include "world/level/tile/TransparentTile.h"

#include "world/phys/Vec3.h"
#include <array>

class LiquidTile : public TransparentTile
{
public:
	LiquidTile(int_t id, int_t tex, const Material &material);

	virtual AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	virtual void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;

	virtual bool isCubeShaped() override;
	virtual Shape getRenderShape() override;
	virtual float getBrightness(LevelSource &level, int_t x, int_t y, int_t z) override;
	virtual bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	virtual int_t getTexture(Facing face) override;

	virtual bool mayPick(int_t data, bool canPickLiquid) override;

	virtual int_t getResource(int_t data, Random &random) override;
	virtual int_t getResourceCount(Random &random) override;

	virtual int_t getRenderLayer() override;
	virtual int_t getTickDelay() override;
	virtual void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	virtual void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	virtual void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;

	void velocityToAddToEntity(Level &level, int_t x, int_t y, int_t z, Entity &entity, Vec3 &vec);
	Vec3 getFlowVector(Level &level, int_t x, int_t y, int_t z);

	static float getHeight(int_t data);
	static double getSlopeAngle(LevelSource &level, int_t x, int_t y, int_t z, const Material &material);

protected:
	int_t getDepth(Level &level, int_t x, int_t y, int_t z) const;
	int_t getEffectiveFlowDepth(Level &level, int_t x, int_t y, int_t z);
	int_t getRenderedDepth(LevelSource &level, int_t x, int_t y, int_t z) const;
	void updateLiquid(Level &level, int_t x, int_t y, int_t z);
	void fizz(Level &level, int_t x, int_t y, int_t z);
};

class LiquidTileDynamic : public LiquidTile
{
public:
	LiquidTileDynamic(int_t id, int_t tex, const Material &material);

	virtual void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	virtual void onPlace(Level &level, int_t x, int_t y, int_t z) override;

private:
	int_t adjacentSourceCount = 0;
	std::array<bool, 4> spreadResult = {};
	std::array<int_t, 4> spreadDistance = {};

	void setStatic(Level &level, int_t x, int_t y, int_t z);
	void trySpreadTo(Level &level, int_t x, int_t y, int_t z, int_t depth);
	int_t getSlopeDistance(Level &level, int_t x, int_t y, int_t z, int_t distance, int_t fromDirection);
	
	
	
	const std::array<bool, 4> &getSpread(Level &level, int_t x, int_t y, int_t z);
	bool blocksLiquidFlow(Level &level, int_t x, int_t y, int_t z);
	int_t getHighestNeighborDepth(Level &level, int_t x, int_t y, int_t z, int_t currentDepth);
	bool canSpreadTo(Level &level, int_t x, int_t y, int_t z);
};

class LiquidTileStatic : public LiquidTile
{
public:
	LiquidTileStatic(int_t id, int_t tex, const Material &material);

	virtual void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	virtual void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;

private:
	void setDynamic(Level &level, int_t x, int_t y, int_t z);
	bool isFlammable(Level &level, int_t x, int_t y, int_t z);
};