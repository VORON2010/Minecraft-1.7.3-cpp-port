#pragma once

#include "world/entity/monster/Monster.h"
#include "world/item/ItemInstance.h"

class Skeleton : public Monster
{
public:
	void playStepSound(int_t x, int_t y, int_t z, int_t tile) override;
	bool isUndead() override { return true; }
	Skeleton(Level &level);
	jstring getEncodeId() const override { return u"Skeleton"; }
	void aiStep() override;
	ItemInstance *getCarriedItem() override;
	void die(Entity *source) override;

protected:
	void checkHurtTarget(Entity &entity, float distance) override;
	void dropDeathLoot() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
};
