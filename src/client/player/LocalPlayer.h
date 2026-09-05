#pragma once

#include "client/player/Input.h"

#include "world/entity/player/Player.h"

#include "nbt/CompoundTag.h"

#include "java/Type.h"

class Minecraft;
class User;

class FurnaceTileEntity;
class DispenserTileEntity;
class ChestTileEntity;
class EntityMinecart;
class CompoundContainer;
class IInventory;
class SignTileEntity;
class LocalPlayer : public Player
{
public:
	std::unique_ptr<Input> input;

protected:
	Minecraft &minecraft;

public:
	int_t changingDimensionDelay = 20;

	float portalTime = 0.0f;
	float oPortalTime = 0.0f;

private:
	bool isInsidePortal = false;

	int_t jumpTriggerTime = 0;
	bool flyActive = false;

public:
	LocalPlayer(Minecraft &minecraft, Level &level, User *user, int_t dimension);
	void addStat(const StatBase &stat, int_t amount) override;
	bool hurt(Entity *source, int_t dmg) override;

	void updateAi() override;
	void handleInsidePortal() override;
	void aiStep() override;

	void releaseAllKeys();
	void setKey(int_t eventKey, bool eventKeyState);

	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;

	void closeContainer() override;
	void take(Entity &entity, int_t count) override;
	void respawn() override;
	void startCrafting(int_t x, int_t y, int_t z);
	void startChest(std::shared_ptr<ChestTileEntity> chest);
	void startChest(std::shared_ptr<CompoundContainer> chest);
	void startChest(std::shared_ptr<EntityMinecart> chest);
	void startChest(std::shared_ptr<IInventory> chest);
	void startFurnace(std::shared_ptr<FurnaceTileEntity> furnace);
	void startDispenser(std::shared_ptr<DispenserTileEntity> dispenser);
	void openTextEdit(std::shared_ptr<SignTileEntity> sign);
	
	void prepareForTick();
	bool isSneaking() override;
	void displayClientMessage(const jstring &message) override;
	void sendChatMessage(const jstring &message) override;
};
