#pragma once

#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/TileEntity.h"

// 1.0 EnchantingTableBlock (116)
class EnchantmentTableTile : public Tile
{
public:
	explicit EnchantmentTableTile(int_t id);

	bool isCubeShaped() override { return false; }
	bool isSolidRender() override { return false; }
	int_t getTexture(Facing face, int_t data) override;
	int_t getTexture(Facing face) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
};

// 1.0 EnchantingTableBlockEntity: the floating book turns to the nearest player
class EnchantmentTableTileEntity : public TileEntity
{
	static Random random;

public:
	int_t time = 0;
	float flip = 0.0f, oFlip = 0.0f, flipT = 0.0f, flipA = 0.0f;
	float open = 0.0f, oOpen = 0.0f;
	float rot = 0.0f, oRot = 0.0f, tRot = 0.0f;

	jstring getEncodeId() const override { return u"EnchantTable"; }
	void tick() override;
};
