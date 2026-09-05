#include "world/item/ItemSword.h"

#include "world/item/ItemInstance.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/WebTile.h"

ItemSword::ItemSword(int_t baseId, ToolMaterialType material) : Item(baseId)
{
	const ToolMaterialData &toolData = getToolMaterialData(material);
	setMaxStackSize(1);
	setMaxDamage(toolData.maxUses);
	weaponDamage = 4 + toolData.damageVsEntity * 2;
}

float ItemSword::getDestroySpeed(const ItemInstance &stack, Tile &tile) const
{
	(void)stack;
	return tile.id == Tile::cobweb.id ? 15.0f : 1.5f;
}

bool ItemSword::canDestroySpecial(const ItemInstance &stack, Tile &tile) const
{
	(void)stack;
	return tile.id == Tile::cobweb.id;
}

int_t ItemSword::getAttackDamage(const ItemInstance &stack, Entity &entity) const
{
	(void)stack;
	(void)entity;
	return weaponDamage;
}

bool ItemSword::hurtEnemy(ItemInstance &stack, Entity &target, Entity &attacker) const
{
	(void)target;
	(void)attacker;
	stack.damageItem(1, attacker);
	return true;
}

bool ItemSword::mineBlock(ItemInstance &stack, int_t tile, int_t x, int_t y, int_t z, Entity &miner) const
{
	(void)tile;
	(void)x;
	(void)y;
	(void)z;
	(void)miner;
	stack.damageItem(2, miner);
	return true;
}
