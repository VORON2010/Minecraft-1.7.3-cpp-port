#pragma once

#include "world/entity/monster/Monster.h"
#include "world/item/ItemInstance.h"

class Skeleton : public Monster
{
public:
	Skeleton(Level &level);
	jstring getEncodeId() const override { return u"Skeleton"; }
	void aiStep() override;
	ItemInstance *getCarriedItem() override;

protected:
	void checkHurtTarget(Entity &entity, float distance) override;
	void dropDeathLoot() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
};
