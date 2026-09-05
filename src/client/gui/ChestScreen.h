#pragma once

#include <memory>

#include "client/gui/ContainerScreen.h"
#include "world/item/ItemInstance.h"

class CompoundContainer;
class ChestTileEntity;
class EntityMinecart;
class BasicInventory;
class Player;
class Container;

class ChestScreen : public ContainerScreen
{
private:
	std::shared_ptr<ChestTileEntity> chest;
	std::shared_ptr<CompoundContainer> compoundChest;
	std::shared_ptr<EntityMinecart> chestMinecart;
	std::shared_ptr<BasicInventory> basicChest;
	std::shared_ptr<Container> containerMenu;
	int_t inventoryRows = 0;
	int_t imageHeight = 0;
	float xMouse = 0.0f;
	float yMouse = 0.0f;

	static constexpr int_t imageWidth = 176;

	int_t getGuiLeft() const;
	int_t getGuiTop() const;
	int_t getChestSize() const;
	int_t getChestSlotX(int_t slot) const;
	int_t getChestSlotY(int_t slot) const;
	int_t getInventorySlotX(int_t slot) const;
	int_t getInventorySlotY(int_t slot) const;
	int_t getSlotAt(int_t x, int_t y) const;
	bool canUseChest(Player &player) const;
	jstring getChestName() const;
	ItemInstance &getChestItem(int_t slot);
	const ItemInstance &getChestItem(int_t slot) const;
	void setChestChanged();
	void renderBg();
	void renderLabels();
	void renderSlot(ItemInstance &stack, int_t x, int_t y, float a);
	const ItemInstance *getSlotItem(int_t slot) const;
	void handleRegularSlotClick(ItemInstance &slotStack, int_t buttonNum);
	void handleSlotClick(int_t slot, int_t buttonNum);
	bool isPointInSlot(int_t relX, int_t relY, int_t slotX, int_t slotY) const;

public:
	ChestScreen(Minecraft &minecraft, std::shared_ptr<ChestTileEntity> chest);
	ChestScreen(Minecraft &minecraft, std::shared_ptr<CompoundContainer> chest);
	ChestScreen(Minecraft &minecraft, std::shared_ptr<EntityMinecart> chest);
	ChestScreen(Minecraft &minecraft, std::shared_ptr<BasicInventory> chest);

	void tick() override;
	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override;
	void removed() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
};
