#pragma once

#include <array>

#include "java/String.h"
#include "java/Type.h"
#include "java/Random.h"
#include "Facing.h"

class ItemInstance;
class Tile;
class Entity;
class Mob;
class Player;
class Level;

class Item
{
public:
	static std::array<Item *, 32000> items;
	static std::array<bool, 32000> subtypeItems;

protected:
	static Random itemRandom;

private:
	int_t baseId = 0;
	int_t shiftedIndex = 0;
	int_t maxStackSize = 64;
	jstring descriptionId;
	Item *containerItem = nullptr;

public:
	explicit Item(int_t baseId);
	virtual ~Item() {}

	int_t getShiftedIndex() const { return shiftedIndex; }
	int_t getMaxStackSize() const { return maxStackSize; }
	int_t getMaxDamage() const { return maxDamage; }
	bool getHasSubtypes() const { return subtypeItems[shiftedIndex]; }
	const jstring &getDescriptionId() const { return descriptionId; }

	Item &setMaxStackSize(int_t size);
	Item &setMaxDamage(int_t damage);
	Item &setHasSubtypes(bool value);
	Item &setIconIndex(int_t icon);
	Item &setDescriptionId(const jstring &id);
	Item &setContainerItem(Item &item);
	bool hasContainerItem() const { return containerItem != nullptr; }
	Item *getContainerItem() const { return containerItem; }

	virtual int_t getIcon(const ItemInstance &stack) const;

	
	protected:
		int_t iconIndex = 0;
		int_t maxDamage = 0;

	public:
	virtual jstring getDescriptionId(const ItemInstance &stack) const;
	virtual float getDestroySpeed(const ItemInstance &stack, Tile &tile) const;
	virtual bool canDestroySpecial(const ItemInstance &stack, Tile &tile) const;
	virtual int_t getAttackDamage(const ItemInstance &stack, Entity &entity) const;
	virtual void use(ItemInstance &stack, Level &level, Player &player) const;
	virtual bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const;
	virtual bool hurtEnemy(ItemInstance &stack, Entity &target, Entity &attacker) const;
	virtual void saddleEntity(ItemInstance &stack, Mob &target) const;
	virtual bool mineBlock(ItemInstance &stack, int_t tile, int_t x, int_t y, int_t z, Entity &miner) const;
	virtual bool isFull3D() const { return false; }
	virtual bool shouldRotateAroundWhenRendering() const { return false; }
	virtual void onUpdate(ItemInstance &stack, Level &level, Entity &entity, int_t slot, bool isHeld);
	virtual void onCreated(ItemInstance &stack, Level &level, Player &player);
};
