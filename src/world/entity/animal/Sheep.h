#pragma once

#include "world/entity/animal/Animal.h"

class Sheep : public Animal
{
public:
	Sheep(Level &level);
	jstring getEncodeId() const override { return u"Sheep"; }
	bool interact(Player &player) override;

protected:
	void dropDeathLoot() override;
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;

public:
	int_t getFleeceColor() const;
	void setFleeceColor(int_t color);
	bool isSheared() const;
	void setSheared(bool sheared);
	static int_t getRandomFleeceColor(Random &random);
};
