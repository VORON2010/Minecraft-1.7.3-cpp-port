#pragma once

#include <memory>

#include "client/gui/ContainerScreen.h"
#include "world/item/ItemInstance.h"

class Minecraft;
class DispenserTileEntity;
class Container;

class DispenserScreen : public ContainerScreen
{
private:
	std::shared_ptr<DispenserTileEntity> dispenser;
	std::shared_ptr<Container> containerMenu;
	int_t hoveredSlot = -1;
	int_t hoveredSlotX = 0;
	int_t hoveredSlotY = 0;
	float xMouse = 0.0f;
	float yMouse = 0.0f;
	static constexpr int_t imageWidth = 176;
	static constexpr int_t imageHeight = 166;

	int_t getGuiLeft() const;
	int_t getGuiTop() const;
	int_t getDispenserSlotX(int_t slot) const;
	int_t getDispenserSlotY(int_t slot) const;
	int_t getInventorySlotX(int_t slot) const;
	int_t getInventorySlotY(int_t slot) const;
	int_t getSlotAt(int_t x, int_t y) const;
	const ItemInstance *getSlotItem(int_t slot) const;
	void renderBg();
	void renderLabels();
	void renderSlot(ItemInstance &stack, int_t x, int_t y, float a);
	void handleRegularSlotClick(ItemInstance &slotStack, int_t buttonNum);
	void handleSlotClick(int_t slot, int_t buttonNum);
	bool isPointInSlot(int_t relX, int_t relY, int_t slotX, int_t slotY) const;

public:
	DispenserScreen(Minecraft &minecraft, std::shared_ptr<DispenserTileEntity> dispenser);

	void render(int_t xm, int_t ym, float a) override;
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
	bool isPauseScreen() override;
	void removed() override;
};
