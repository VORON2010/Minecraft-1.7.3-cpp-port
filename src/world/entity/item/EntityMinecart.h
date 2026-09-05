#pragma once

#include <array>
#include "world/item/ItemInstance.h"
#include "world/entity/Entity.h"
#include "world/inventory/IInventory.h"

class Vec3;

class EntityMinecart : public Entity, public IInventory
{
public:
	static constexpr int_t TYPE_RIDEABLE = 0;
	static constexpr int_t TYPE_CHEST = 1;
	static constexpr int_t TYPE_FURNACE = 2;

private:
	bool flipped = false;
	int_t lerpSteps = 0;
	double lerpX = 0.0;
	double lerpY = 0.0;
	double lerpZ = 0.0;
	float lerpYaw = 0.0f;
	float lerpPitch = 0.0f;
	std::array<ItemInstance, 27> cargoItems = {};

public:
	int_t minecartCurrentDamage = 0;
	int_t minecartTimeSinceHit = 0;
	int_t minecartRockDirection = 1;
	int_t minecartType = TYPE_RIDEABLE;
	int_t fuel = 0;
	double pushX = 0.0;
	double pushZ = 0.0;

	EntityMinecart(Level &level);
	EntityMinecart(Level &level, double x, double y, double z, int_t minecartType);

	jstring getEncodeId() const override { return u"Minecart"; }
	bool isPickable() override { return !removed; }
	bool isPushable() override { return true; }
	AABB *getCollideAgainstBox(Entity &entity) override;
	double getRideHeight() override;
	bool hurt(Entity *source, int_t dmg) override;
	bool interact(Player &player) override;
	void tick() override;
	void animateHurt() override;
	void lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps) override;

	Vec3 *getPosOffs(double x, double y, double z, double offset) const;
	Vec3 *getPosOnTrack(double x, double y, double z) const;

	ItemInstance &getItem(int_t slot);
	const ItemInstance &getItem(int_t slot) const;
	void setItem(int_t slot, const ItemInstance &item);
	ItemInstance removeItem(int_t slot, int_t count);
	int_t getContainerSize() const;
	bool canUse(Player &player) const;
	jstring getName() const;

	int_t getSizeInventory() const override { return getContainerSize(); }
	ItemInstance *getStackInSlot(int_t slot) override { return getItem(slot).isEmpty() ? nullptr : &getItem(slot); }
	const ItemInstance *getStackInSlot(int_t slot) const override { return getItem(slot).isEmpty() ? nullptr : &getItem(slot); }
	ItemInstance decrStackSize(int_t slot, int_t count) override { return removeItem(slot, count); }
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override { setItem(slot, item); }
	jstring getInvName() const override { return getName(); }
	void onInventoryChanged() override {}
	bool canInteractWith(Player &player) const override { return canUse(player); }

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
