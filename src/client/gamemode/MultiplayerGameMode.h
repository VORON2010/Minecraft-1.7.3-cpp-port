#pragma once

#include "client/gamemode/GameMode.h"

class NetClientHandler;

class MultiplayerGameMode : public GameMode
{
private:
	int_t xDestroyBlock = -1;
	int_t yDestroyBlock = -1;
	int_t zDestroyBlock = -1;

	float destroyProgress = 0.0f;
	float oDestroyProgress = 0.0f;
	float destroyTicks = 0.0f;
	int_t destroyDelay = 0;
	bool isDestroying = false;

	NetClientHandler &netClientHandler;
	int_t currentPlayerItem = 0;

	void syncCurrentPlayItem();

public:
	MultiplayerGameMode(Minecraft &minecraft, NetClientHandler &netClientHandler);

	void initPlayer(std::shared_ptr<Player> player) override;

	bool destroyBlock(int_t x, int_t y, int_t z, Facing face) override;
	void startDestroyBlock(int_t x, int_t y, int_t z, Facing face) override;
	void stopDestroyBlock() override;
	void continueDestroyBlock(int_t x, int_t y, int_t z, Facing face) override;

	void render(float a) override;

	float getPickRange() override;

	void initLevel(std::shared_ptr<Level> level) override;

	void tick() override;

	bool useItemOn(std::shared_ptr<Player> &player, Level &level, ItemInstance *item,
		int_t x, int_t y, int_t z, Facing face) override;
	bool useItem(std::shared_ptr<Player> &player, Level &level, ItemInstance *item) override;

	std::shared_ptr<Player> createPlayer(Level &level) override;

	void attack(std::shared_ptr<Player> &player, std::shared_ptr<Entity> &entity) override;
	void interact(std::shared_ptr<Player> &player, std::shared_ptr<Entity> &entity) override;
	std::unique_ptr<ItemInstance> clickContainer(int_t windowId, int_t slot, int_t button,
		bool shiftClick, Player &player) override;
	void closeContainer(int_t windowId, Player &player) override;
};
