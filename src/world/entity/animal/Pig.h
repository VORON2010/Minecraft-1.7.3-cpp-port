#pragma once

#include "world/entity/animal/Animal.h"

class Pig : public Animal
{
public:
	Pig(Level &level);
	jstring getEncodeId() const override { return u"Pig"; }
	bool interact(Player &player) override;
	void onStruckByLightning(Entity &lightning) override;

protected:
	void causeFallDamage(float distance) override;
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;

public:
	bool isSaddled() const;
	void setSaddled(bool saddled);
};
