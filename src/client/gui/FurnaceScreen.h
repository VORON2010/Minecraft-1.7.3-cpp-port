#pragma once

#include <memory>

#include "client/gui/ContainerScreen.h"

class FurnaceTileEntity;
class ItemInstance;
class Container;

class FurnaceScreen : public ContainerScreen
{
private:
	std::shared_ptr<FurnaceTileEntity> furnace;
	std::shared_ptr<Container> containerMenu;
	float xMouse = 0.0f;
	float yMouse = 0.0f;

	static constexpr int_t imageWidth = 176;
	static constexpr int_t imageHeight = 166;

	int_t getGuiLeft() const;
	int_t getGuiTop() const;
	int_t getInventorySlotX(int_t slot) const;
	int_t getInventorySlotY(int_t slot) const;
	int_t getSlotAt(int_t x, int_t y) const;
	void renderBg();
	void renderLabels();
	void renderSlot(ItemInstance &stack, int_t x, int_t y, float a);
	const ItemInstance *getSlotItem(int_t slot) const;
	void handleRegularSlotClick(ItemInstance &slotStack, int_t buttonNum);
	void handleOutputSlotClick(int_t buttonNum);

public:
	explicit FurnaceScreen(Minecraft &minecraft, std::shared_ptr<FurnaceTileEntity> furnace);

	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override;
	void removed() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
};
