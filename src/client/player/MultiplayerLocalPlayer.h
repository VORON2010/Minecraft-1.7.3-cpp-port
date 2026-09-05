#pragma once

#include "client/player/LocalPlayer.h"

class NetClientHandler;

class MultiplayerLocalPlayer : public LocalPlayer
{
private:
	int_t inventoryUpdateTicks = 0;
	bool receivedInitialHealth = false;
	double lastSentX = 0.0;
	double lastSentStance = 0.0;
	double lastSentY = 0.0;
	double lastSentZ = 0.0;
	float lastSentYRot = 0.0f;
	float lastSentXRot = 0.0f;
	bool lastSentOnGround = false;
	bool wasSneaking = false;
	int_t ticksSinceLastMovePacket = 0;

	void sendInventoryChanged();

protected:
	void actuallyHurt(int_t dmg) override;

public:
	NetClientHandler &sendQueue;

	MultiplayerLocalPlayer(Minecraft &minecraft, Level &level, User *user, NetClientHandler &sendQueue);

	bool hurt(Entity *source, int_t dmg) override;
	void heal(int_t amount) override;
	void tick() override;
	void sendMotionUpdates();
	void drop() override;
	void sendChatMessage(const jstring &message) override;
	void swing() override;
	void respawn() override;
	void closeContainer() override;
	void setHealth(int_t health);
	void addStat(const StatBase &stat, int_t amount) override;
	void awardServerStat(const StatBase &stat, int_t amount);
protected:
	void reallyDrop(std::shared_ptr<EntityItem> itemEntity) override;
};
