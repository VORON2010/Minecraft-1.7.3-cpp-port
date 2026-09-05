#pragma once

#include "world/entity/Entity.h"
#include "world/item/ItemInstance.h"

class Player;

class EntityItem : public Entity
{
	virtual jstring getEncodeId() const override { return u"Item"; }

public:
	ItemInstance item;
	int_t age = 0;
	float bobOffs = 0.0f;
	int_t throwTime = 10;

	EntityItem(Level &level);
	EntityItem(Level &level, double x, double y, double z, const ItemInstance &stack);

	void tick() override;
	void playerTouch(Player &player) override;
	bool hurt(Entity *source, int_t dmg) override;
	bool shouldRenderAtSqrDistance(double distance) override;
	bool handleWaterMovement() override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;

private:
	int_t health = 5;
};
