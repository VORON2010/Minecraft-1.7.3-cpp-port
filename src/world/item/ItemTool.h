#pragma once

#include <cstddef>

#include "world/item/Item.h"

enum class ToolMaterialType
{
	WOOD,
	STONE,
	IRON,
	DIAMOND,
	GOLD,
};

struct ToolMaterialData
{
	int_t harvestLevel;
	int_t maxUses;
	float efficiencyOnProperMaterial;
	int_t damageVsEntity;
};

const ToolMaterialData &getToolMaterialData(ToolMaterialType material);

class ItemTool : public Item
{
public:
	ItemTool(int_t baseId, int_t baseDamage, ToolMaterialType material, const int_t *effectiveTiles, std::size_t effectiveTileCount);

	float getDestroySpeed(const ItemInstance &stack, Tile &tile) const override;
	int_t getAttackDamage(const ItemInstance &stack, Entity &entity) const override;
	bool hurtEnemy(ItemInstance &stack, Entity &target, Entity &attacker) const override;
	bool mineBlock(ItemInstance &stack, int_t tile, int_t x, int_t y, int_t z, Entity &miner) const override;
	bool isFull3D() const override { return true; }

protected:
	bool isEffectiveAgainst(Tile &tile) const;
	ToolMaterialType toolMaterial;

private:
	const int_t *effectiveTiles = nullptr;
	std::size_t effectiveTileCount = 0;
	float efficiencyOnProperMaterial = 1.0f;
	int_t damageVsEntity = 1;
};
