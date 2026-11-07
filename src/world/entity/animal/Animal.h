#pragma once

#include "world/entity/PathfinderMob.h"

class ItemInstance;

// 1.0 AnimalEntity: age (negative for babies), love from wheat, breeding
class Animal : public PathfinderMob
{
	int_t inLove = 0;
	int_t loveTime = 0;

public:
	int_t age = 0;

	Animal(Level &level);

	int_t getAge() const { return age; }
	void setAge(int_t value) { age = value; }
	bool isBaby() override { return age < 0; }
	bool isInLove() const { return inLove > 0; }
	void setInLove() { inLove = 600; }
	bool interact(Player &player) override;
	bool hurt(Entity *source, int_t dmg) override;
	int_t getAmbientSoundInterval() override;
	bool canSpawn() override;
	virtual bool isFood(const ItemInstance &item) const;
	virtual std::shared_ptr<Animal> getBreedOffspring(Animal &partner) { (void)partner; return nullptr; }

protected:
	int_t getExperienceReward() override { return 1 + random.nextInt(3); }
	void updateAi() override;
	void checkHurtTarget(Entity &entity, float distance) override;
	void attackBlockedEntity(Entity &entity, float distance) override { (void)entity; (void)distance; }
	std::shared_ptr<Entity> findAttackTarget() override;
	float getWalkTargetValue(int_t x, int_t y, int_t z) override;
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;

private:
	void breed(Animal &partner);
	void hearts(int_t count, bool moving);
};
