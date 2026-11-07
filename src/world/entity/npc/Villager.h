#pragma once

#include "world/entity/PathfinderMob.h"

// 1.0 villager: wanders around villages, profession picks the robe
class Villager : public PathfinderMob
{
private:
	int_t profession = 0;

public:
	Villager(Level &level);
	Villager(Level &level, int_t profession);
	jstring getEncodeId() const override { return u"Villager"; }
	int_t getProfession() const { return profession; }

	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;

protected:
	bool canDespawn() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;

private:
	void updateTexture();
};
