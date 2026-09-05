#pragma once

#include <vector>

#include "client/gui/ContainerScreen.h"
#include "world/item/ItemInstance.h"

class Container;

class InventoryScreen : public ContainerScreen
{
private:
	std::shared_ptr<Container> containerMenu;
	float xMouse = 0.0f;
	float yMouse = 0.0f;
	std::vector<ItemInstance> craftingSlots;
	ItemInstance craftingResult;
	int_t craftingWidth = 0;
	int_t craftingHeight = 0;

	static constexpr int_t imageWidth = 176;
	static constexpr int_t imageHeight = 166;

	int_t getGuiLeft() const;
	int_t getGuiTop() const;
	int_t getInventorySlotX(int_t slot) const;
	int_t getInventorySlotY(int_t slot) const;
	int_t getCraftingSlotX(int_t slot) const;
	int_t getCraftingSlotY(int_t slot) const;
	int_t getSlotAt(int_t x, int_t y) const;
	int_t getArmorSlotX(int_t slot) const;
	int_t getArmorSlotY(int_t slot) const;
	bool isArmorSlot(int_t slot) const;
	int_t armorSlotToArmorIndex(int_t slot) const;

	void updateCraftingResult();
	void syncCraftingSlotsFromMenu();
	void consumeCraftingIngredients();
	void dropCraftingContents();
	void renderBg(float a);
	virtual void renderLabels();
	void renderPlayerModel(int_t x, int_t y, int_t scale);
	void renderSlot(ItemInstance &stack, int_t x, int_t y, float a);
	const ItemInstance *getSlotItem(int_t slot) const;
	void handleRegularSlotClick(ItemInstance &slotStack, int_t buttonNum);
	void handleSlotClick(int_t slot, int_t buttonNum);

protected:
	virtual jstring getBackgroundTexture() const;
	virtual int_t getCraftingGridLeft() const;
	virtual int_t getCraftingGridTop() const;
	virtual int_t getResultSlotX() const;
	virtual int_t getResultSlotY() const;
	virtual int_t getTitleX() const;
	virtual int_t getTitleY() const;
	virtual jstring getTitleText() const;
	virtual bool shouldRenderPlayerModel() const;

public:
	InventoryScreen(Minecraft &minecraft, int_t craftingWidth = 2, int_t craftingHeight = 2);

	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override;
	void removed() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
};
