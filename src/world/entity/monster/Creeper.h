#pragma once

#include "world/entity/monster/Monster.h"

class Creeper : public Monster
{
private:
	int_t swell = 0;
	int_t oldSwell = 0;
	int_t getCreeperState() const;
	void setCreeperState(int_t state);

public:
	Creeper(Level &level);
	jstring getEncodeId() const override { return u"Creeper"; }
	void tick() override;
	float getSwelling(float a) const;
	bool isPowered() const;
	void setPowered(bool powered); 
	void onStruckByLightning(Entity &lightning) override;
	void die(Entity *source) override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
	void attackBlockedEntity(Entity &entity, float distance) override;
	void checkHurtTarget(Entity &entity, float distance) override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
};
