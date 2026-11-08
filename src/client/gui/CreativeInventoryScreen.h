#pragma once

#include <memory>
#include <vector>

#include "client/gui/Screen.h"
#include "world/item/ItemInstance.h"

class GuiTextField;

// 1.5.2 CreativeInventoryScreen: twelve tabs, a search box and the survival inventory tab
class CreativeInventoryScreen : public Screen
{
	static int_t selectedTab;

	std::vector<ItemInstance> items;
	float scrollOffs = 0.0f;
	bool scrolling = false;
	bool wasClicking = false;
	std::shared_ptr<GuiTextField> searchBox;
	int_t leftPos = 0;
	int_t topPos = 0;

	static constexpr int_t imageWidth = 195;
	static constexpr int_t imageHeight = 136;

	// a slot of the screen: one of the 45 list cells, a hotbar cell, a player menu slot, or the bin
	struct SlotRef
	{
		enum Kind { NONE, LIST, HOTBAR, PLAYER, BIN } kind = NONE;
		int_t index = 0;
		int_t x = 0;
		int_t y = 0;
	};

	void selectTab(int_t tab);
	void refreshSearch();
	bool canScroll() const;
	int_t scrollRow() const;
	ItemInstance *listItem(int_t cell);
	std::vector<SlotRef> getSlots() const;
	SlotRef findSlot(int_t mx, int_t my) const;
	ItemInstance *slotItem(const SlotRef &slot);
	void slotClicked(const SlotRef &slot, int_t button, bool shift, bool outside);
	bool checkTabClicked(int_t tab, int_t mx, int_t my) const;
	void renderTab(int_t tab);
	bool renderTabTooltip(int_t tab, int_t mx, int_t my);
	void renderPlayer(int_t x, int_t y, int_t scale, float xd, float yd);
	void dropFromMenu(const ItemInstance &stack);

public:
	explicit CreativeInventoryScreen(Minecraft &minecraft);

	void init() override;
	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override { return false; }

protected:
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
	void mouseReleased(int_t x, int_t y, int_t buttonNum) override;
	void mouseScrolled(int_t x, int_t y, int_t scrollAmount) override;
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
};
