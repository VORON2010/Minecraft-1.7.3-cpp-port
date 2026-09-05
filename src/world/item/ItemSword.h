#pragma once

#include "world/item/Item.h"
#include "world/item/ItemTool.h"

class ItemSword : public Item
{
public:
	ItemSword(int_t baseId, ToolMaterialType material);

	float getDestroySpeed(const ItemInstance &stack, Tile &tile) const override;
	bool canDestroySpecial(const ItemInstance &stack, Tile &tile) const override;
	int_t getAttackDamage(const ItemInstance &stack, Entity &entity) const override;
	bool hurtEnemy(ItemInstance &stack, Entity &target, Entity &attacker) const override;
	bool mineBlock(ItemInstance &stack, int_t tile, int_t x, int_t y, int_t z, Entity &miner) const override;
	bool isFull3D() const override { return true; }

private:
	int_t weaponDamage = 1;
};
