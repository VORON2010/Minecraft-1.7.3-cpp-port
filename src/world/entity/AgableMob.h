#pragma once

#include "world/entity/PathfinderMob.h"

// 1.5.2 PassiveEntity (Mojang AgableMob): breeding age, babies are half size,
// a matching spawn egg used on it makes a baby
class AgableMob : public PathfinderMob
{
	float registeredBBWidth = -1.0f;
	float registeredBBHeight = 0.0f;
	int_t age = 0;

	void updateSize(float scale);

public:
	explicit AgableMob(Level &level) : PathfinderMob(level) {}

	virtual std::shared_ptr<AgableMob> getBreedOffspring(AgableMob &partner) = 0;
	bool interact(Player &player) override;
	int_t getAge() const { return age; }
	void setAge(int_t value);
	bool isBaby() override { return age < 0; }
	void setBaby(bool baby) { updateSize(baby ? 0.5f : 1.0f); }
	void aiStep() override;

protected:
	void setSize(float width, float height) override;
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
