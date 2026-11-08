#pragma once

#include <memory>

#include "world/entity/AgableMob.h"
#include "world/entity/npc/Merchant.h"

class Village;

// 1.5.2 VillagerEntity: goal AI, professions (0 farmer, 1 librarian, 2 priest, 3 smith, 4 butcher)
// and emerald trading
class Villager : public AgableMob, public Merchant
{
	int_t profession = 0;
	bool inLove = false;
	bool chasing = false;
	Village *village = nullptr;
	Player *tradingPlayer = nullptr;
	std::unique_ptr<MerchantRecipeList> offers;
	int_t updateMerchantTimer = 0;
	bool addRecipeOnUpdate = false;
	int_t riches = 0;
	jstring lastPlayerTradeName;
	bool hasLastPlayerTradeName = false;
	bool rewardPlayersOnFirstVillage = false;
	float baseRecipeChanceMod = 0.0f;
	int_t villageUpdateInterval = 0;

	float getRecipeChance(float baseChance);
	void addOffers(int_t count);
	void addParticlesAroundSelf(const jstring &name);

public:
	Villager(Level &level);
	Villager(Level &level, int_t profession);
	jstring getEncodeId() const override { return u"Villager"; }

	bool useNewAi() override { return true; }
	bool interact(Player &player) override;
	jstring getTexture() override;

	void setProfession(int_t value) { profession = value; }
	int_t getProfession() const { return profession; }
	bool isInLove() const { return inLove; }
	void setInLove(bool value) { inLove = value; }
	void setChasing(bool value) { chasing = value; }
	bool isChasing() const { return chasing; }
	bool isTrading() const { return tradingPlayer != nullptr; }
	void die(Entity *source) override;
	void setEntityVariety() override;
	void setRewardPlayersInVillage() { rewardPlayersOnFirstVillage = true; }
	void sendEntityEvent(byte_t event);
	void handleEntityEvent(byte_t event) override;
	bool hurt(Entity *source, int_t dmg) override;

	std::shared_ptr<AgableMob> getBreedOffspring(AgableMob &partner) override;
	std::shared_ptr<Villager> getBreedOffspringVillager(AgableMob &partner);

	// Merchant
	void setTradingPlayer(Player *player) override { tradingPlayer = player; }
	Player *getTradingPlayer() override { return tradingPlayer; }
	MerchantRecipeList *getOffers(Player &player) override;
	void notifyTrade(MerchantRecipe &recipe) override;

	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;

protected:
	void serverAiMobStep() override;
	bool canDespawn() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
};
