#include "world/item/Item.h"

#include <stdexcept>
#include <string>

#include "world/item/ItemInstance.h"

std::array<Item *, 32000> Item::items = {};
std::array<bool, 32000> Item::subtypeItems = {};
Random Item::itemRandom;

Item::Item(int_t baseId)
{
	this->baseId = baseId;
	shiftedIndex = 256 + baseId;
	if (shiftedIndex < 0 || shiftedIndex >= static_cast<int_t>(items.size()))
		throw std::runtime_error("Item id out of range: " + std::to_string(baseId));
	if (items[shiftedIndex] != nullptr)
		throw std::runtime_error("Item slot already occupied: " + std::to_string(shiftedIndex));
	items[shiftedIndex] = this;
}

Item &Item::setMaxStackSize(int_t size)
{
	maxStackSize = size;
	return *this;
}

Item &Item::setMaxDamage(int_t damage)
{
	maxDamage = damage;
	return *this;
}

Item &Item::setHasSubtypes(bool value)
{
	subtypeItems[shiftedIndex] = value;
	return *this;
}

Item &Item::setIconIndex(int_t icon)
{
	iconIndex = icon;
	return *this;
}

Item &Item::setDescriptionId(const jstring &id)
{
	descriptionId = id;
	return *this;
}

int_t Item::getIcon(const ItemInstance &stack) const
{
	(void)stack;
	return iconIndex;
}

jstring Item::getDescriptionId(const ItemInstance &stack) const
{
	(void)stack;
	return descriptionId;
}

float Item::getDestroySpeed(const ItemInstance &stack, Tile &tile) const
{
	(void)stack;
	(void)tile;
	return 1.0f;
}

bool Item::canDestroySpecial(const ItemInstance &stack, Tile &tile) const
{
	(void)stack;
	(void)tile;
	return false;
}

int_t Item::getAttackDamage(const ItemInstance &stack, Entity &entity) const
{
	(void)stack;
	(void)entity;
	return 1;
}

void Item::use(ItemInstance &stack, Level &level, Player &player) const
{
	(void)stack;
	(void)level;
	(void)player;
}

bool Item::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)stack;
	(void)player;
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	(void)face;
	return false;
}

bool Item::hurtEnemy(ItemInstance &stack, Entity &target, Entity &attacker) const
{
	(void)stack;
	(void)target;
	(void)attacker;
	return false;
}

void Item::saddleEntity(ItemInstance &stack, Mob &target) const
{
	(void)stack;
	(void)target;
}

bool Item::mineBlock(ItemInstance &stack, int_t tile, int_t x, int_t y, int_t z, Entity &miner) const
{
	(void)stack;
	(void)tile;
	(void)x;
	(void)y;
	(void)z;
	(void)miner;
	return false;
}

void Item::onUpdate(ItemInstance &stack, Level &level, Entity &entity, int_t slot, bool isHeld)
{
	(void)stack;
	(void)level;
	(void)entity;
	(void)slot;
	(void)isHeld;
}

void Item::onCreated(ItemInstance &stack, Level &level, Player &player)
{
	(void)stack;
	(void)level;
	(void)player;
}

Item &Item::setContainerItem(Item &item)
{
	containerItem = &item;
	return *this;
}
