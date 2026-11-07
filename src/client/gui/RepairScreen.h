#pragma once

#include <memory>

#include "client/gui/ContainerScreen.h"
#include "world/item/ItemInstance.h"

class ContainerRepair;
class GuiTextField;

// 1.4.7 AnvilScreen: repair and rename
class RepairScreen : public ContainerScreen
{
	std::shared_ptr<ContainerRepair> menu;
	std::shared_ptr<GuiTextField> nameField;
	ItemInstance lastFirst;

	static constexpr int_t imageWidth = 176;
	static constexpr int_t imageHeight = 166;

	int_t getGuiLeft() const { return (width - imageWidth) / 2; }
	int_t getGuiTop() const { return (height - imageHeight) / 2; }
	int_t getSlotAt(int_t x, int_t y) const;
	void checkFirstSlot();

public:
	RepairScreen(Minecraft &minecraft, std::shared_ptr<ContainerRepair> menu);

	void init() override;
	void tick() override;
	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override { return false; }
	void removed() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
};
