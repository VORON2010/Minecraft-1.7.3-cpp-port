#pragma once

#include <atomic>
#include <memory>
#include <mutex>

#include "java/Type.h"

class Entity;
class Tile;
class Item;
class Mob;
class Player;
class Level;
class CompoundTag;
class ItemInstance;

class ItemInstanceReference
{
public:
	struct Snapshot
	{
		int_t stackSize = 0;
		int_t itemID = 0;
		int_t itemDamage = 0;
	};

	Snapshot get() const;

private:
	friend class ItemInstance;

	explicit ItemInstanceReference(const ItemInstance &item);
	void detach(const ItemInstance &item);
	void move(const ItemInstance &from, const ItemInstance &to);

	mutable std::mutex lock;
	const ItemInstance *liveItem;
	Snapshot detachedItem;
};

enum class Facing;
class ItemInstance
{
private:
	mutable std::shared_ptr<ItemInstanceReference> javaReference;
	void detachJavaReference();
	void moveJavaReference(ItemInstance &other);

public:
	std::atomic<int_t> stackSize{0};
	std::atomic<int_t> itemID{0};
	std::atomic<int_t> itemDamage{0};
	int_t popTime = 0;

	ItemInstance() = default;
	ItemInstance(const ItemInstance &other);
	ItemInstance(ItemInstance &&other) noexcept;
	ItemInstance &operator=(const ItemInstance &other);
	ItemInstance &operator=(ItemInstance &&other) noexcept;
	~ItemInstance();
	ItemInstance(int_t itemID);
	ItemInstance(int_t itemID, int_t count);
	ItemInstance(int_t itemID, int_t count, int_t damage);
	ItemInstance(CompoundTag &tag);

	bool isEmpty() const { return stackSize <= 0 || itemID == 0; }
	Item *getItem() const;
	int_t getMaxStackSize() const;
	int_t getMaxDamage() const;
	bool getHasSubtypes() const;
	bool isStackable() const;
	bool isItemDamaged() const;
	int_t getIcon() const;
	int_t getAuxValue() const { return itemDamage; }
	float getDestroySpeed(Tile &tile) const;
	bool canDestroySpecial(Tile &tile) const;
	int_t getAttackDamage(Entity &entity) const;
	void use(Level &level, Player &player);
	bool useOn(Player &player, Level &level, int_t x, int_t y, int_t z, Facing face);
	void damageItem(int_t amount, Entity &entity);
	bool hurtEnemy(Entity &target, Entity &attacker);
	void saddleEntity(Mob &target);
	bool mineBlock(int_t tile, int_t x, int_t y, int_t z, Entity &miner);
	void onCrafted(Level &level, Player &player);
	void save(CompoundTag &tag) const;
	void load(CompoundTag &tag);
	ItemInstance remove(int_t count);
	bool sameItem(const ItemInstance &other) const;
	std::shared_ptr<const ItemInstanceReference> retainReference() const;
};
