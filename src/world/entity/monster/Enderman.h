#pragma once

#include <array>

#include "world/entity/monster/Monster.h"

class Player;

// b1.8 / 1.0 enderman: gets angry when looked at, teleports, carries blocks
class Enderman : public Monster
{
private:
	static std::array<bool, 256> mayTake;
	static bool mayTakeInitialized;

	int_t carriedTile = 0;
	int_t carriedData = 0;
	int_t aggroTime = 0;
	int_t teleportTime = 0;

public:
	bool creepy = false;

	Enderman(Level &level);
	jstring getEncodeId() const override { return u"Enderman"; }

	void aiStep() override;
	bool hurt(Entity *source, int_t damage) override;

	int_t getCarriedTile() const { return carriedTile; }
	int_t getCarriedData() const { return carriedData; }
	void setCarriedTile(int_t tile) { carriedTile = tile & 0xFF; }
	void setCarriedData(int_t data) { carriedData = data & 0xFF; }

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
	std::shared_ptr<Entity> findAttackTarget() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
	void dropDeathLoot() override;

private:
	static void initMayTake();
	bool isLookingAtMe(Player &player);
	bool teleport();
	bool teleportTowards(Entity &target);
	bool teleport(double x, double y, double z);
};
