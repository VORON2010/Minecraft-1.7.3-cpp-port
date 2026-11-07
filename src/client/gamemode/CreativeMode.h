#pragma once

#include "client/gamemode/GameMode.h"

// 1.0 CreativeInteractionManager
class CreativeMode : public GameMode
{
	int_t destroyDelay = 0;

public:
	CreativeMode(Minecraft &minecraft);

	static void setCreative(Player &player);
	static void setSurvival(Player &player);

	void initPlayer(std::shared_ptr<Player> player) override;
	void adjustPlayer(std::shared_ptr<Player> player) override;

	void startDestroyBlock(int_t x, int_t y, int_t z, Facing face) override;
	void continueDestroyBlock(int_t x, int_t y, int_t z, Facing face) override;
	void stopDestroyBlock() override;
	bool useItemOn(std::shared_ptr<Player> &player, Level &level, ItemInstance *item, int_t x, int_t y, int_t z, Facing face) override;

	float getPickRange() override;
	bool canHurtPlayer() override;

	bool isCreativeMode() const override { return true; }

private:
	void destroyInCreative(int_t x, int_t y, int_t z, Facing face);
};
