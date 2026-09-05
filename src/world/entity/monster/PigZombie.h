#pragma once

#include "world/entity/monster/Zombie.h"
#include "world/item/ItemInstance.h"

class PigZombie : public Zombie
{
private:
	int_t angerTime = 0;
	int_t playAngrySoundIn = 0;

public:
	PigZombie(Level &level);
	jstring getEncodeId() const override { return u"PigZombie"; }
	void tick() override;
	bool hurt(Entity *source, int_t damage) override;
	bool canSpawn() override;
	ItemInstance *getCarriedItem() override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
	std::shared_ptr<Entity> findAttackTarget() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;

private:
	void becomeAngryAt(Entity &target);
};
