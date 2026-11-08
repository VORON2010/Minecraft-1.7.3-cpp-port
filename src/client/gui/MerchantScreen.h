#pragma once

#include <memory>

#include "client/gui/Button.h"
#include "client/gui/ContainerScreen.h"

class MerchantMenu;
class Merchant;

// 1.5.2 VillagerScreen: shows one offer at a time, arrows flip between them
class MerchantScreen : public ContainerScreen
{
	class TradeButton : public Button
	{
		bool forward;

	public:
		TradeButton(int_t id, int_t x, int_t y, bool forward) : Button(id, x, y, 12, 19, u""), forward(forward) {}
		void render(Minecraft &minecraft, int_t xm, int_t ym) override;
	};

	std::shared_ptr<MerchantMenu> menu;
	Merchant &merchant;
	std::shared_ptr<TradeButton> nextButton;
	std::shared_ptr<TradeButton> prevButton;
	int_t currentRecipeIndex = 0;
	jstring title;

	static constexpr int_t imageWidth = 176;
	static constexpr int_t imageHeight = 166;

	int_t getGuiLeft() const { return (width - imageWidth) / 2; }
	int_t getGuiTop() const { return (height - imageHeight) / 2; }
	int_t getSlotAt(int_t x, int_t y) const;
	bool isHovering(int_t x, int_t y, int_t w, int_t h, int_t mx, int_t my) const;

public:
	MerchantScreen(Minecraft &minecraft, std::shared_ptr<MerchantMenu> menu, Merchant &merchant, const jstring &name);

	void init() override;
	void tick() override;
	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override { return false; }
	void removed() override;

protected:
	void buttonClicked(Button &button) override;
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
};
