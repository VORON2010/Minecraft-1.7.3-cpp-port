#pragma once

#include "client/gamemode/GameMode.h"

class CreativeMode : public GameMode
{
public:
	CreativeMode(Minecraft &minecraft);

	virtual void initPlayer(std::shared_ptr<Player> player) override;
	virtual void initLevel(std::shared_ptr<Level> level) override;

	virtual void startDestroyBlock(int_t x, int_t y, int_t z, Facing face) override;
	virtual void continueDestroyBlock(int_t x, int_t y, int_t z, Facing face) override;
	virtual bool destroyBlock(int_t x, int_t y, int_t z, Facing face) override;
	virtual bool useItemOn(std::shared_ptr<Player> &player, Level &level, ItemInstance *item, int_t x, int_t y, int_t z, Facing face) override;
	virtual bool useItem(std::shared_ptr<Player> &player, Level &level, ItemInstance *item) override;
	
	virtual float getPickRange() override;
	virtual bool canHurtPlayer() override;
	
	virtual bool isCreativeMode() const override { return true; }
};
