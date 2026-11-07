#pragma once

#include <memory>

#include "client/gui/ContainerScreen.h"
#include "java/Random.h"
#include "world/item/ItemInstance.h"

class ContainerEnchantment;

// 1.0 EnchantingTableScreen
class EnchantmentScreen : public ContainerScreen
{
	std::shared_ptr<ContainerEnchantment> menu;
	Random random;
	int_t time = 0;
	float flip = 0.0f, oFlip = 0.0f, flipT = 0.0f, flipA = 0.0f;
	float open = 0.0f, oOpen = 0.0f;
	ItemInstance last;

	static constexpr int_t imageWidth = 176;
	static constexpr int_t imageHeight = 166;

	int_t getGuiLeft() const { return (width - imageWidth) / 2; }
	int_t getGuiTop() const { return (height - imageHeight) / 2; }
	int_t getSlotAt(int_t x, int_t y) const;
	void renderBg(int_t xm, int_t ym, float a);
	void renderBook(float a);

public:
	EnchantmentScreen(Minecraft &minecraft, std::shared_ptr<ContainerEnchantment> menu);

	void render(int_t xm, int_t ym, float a) override;
	void tick() override;
	bool isPauseScreen() override { return false; }
	void removed() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
};
