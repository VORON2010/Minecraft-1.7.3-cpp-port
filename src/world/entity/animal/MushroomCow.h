#pragma once

#include "world/entity/animal/Cow.h"
#include "world/entity/PathfinderMob.h"

// 1.0 MooshroomEntity: bowl gives stew, shears turn it into a cow and drop five red mushrooms
class MushroomCow : public Cow
{
public:
	explicit MushroomCow(Level &level);
	jstring getEncodeId() const override { return u"MushroomCow"; }
	bool interact(Player &player) override;
	std::shared_ptr<Animal> getBreedOffspring(Animal &partner) override;
};

// 1.0 SnowGolemEntity: throws snowballs at monsters, leaves snow behind, melts in hot biomes
class SnowMan : public PathfinderMob
{
public:
	explicit SnowMan(Level &level);
	jstring getEncodeId() const override { return u"SnowMan"; }
	int_t getMaxHealth() const { return 4; }

protected:
	void updateAi() override;
	void checkHurtTarget(Entity &target, float distance) override;
	void causeFallDamage(float distance) override { (void)distance; }
	bool canDespawn() override { return false; }
	int_t getAmbientSoundInterval() override { return 120; }
	jstring getAmbientSound() override { return u""; }
	jstring getHurtSound() override { return u""; }
	jstring getDeathSound() override { return u""; }
	int_t getDeathLoot() override;
	void dropDeathLoot() override;
};
