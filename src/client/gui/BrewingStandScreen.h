#pragma once

#include <memory>

#include "client/gui/ContainerScreen.h"

class BrewingStandTileEntity;
class Container;

// 1.0 BrewingStandScreen
class BrewingStandScreen : public ContainerScreen
{
	std::shared_ptr<BrewingStandTileEntity> stand;
	std::shared_ptr<Container> containerMenu;

	static constexpr int_t imageWidth = 176;
	static constexpr int_t imageHeight = 166;

	int_t getGuiLeft() const { return (width - imageWidth) / 2; }
	int_t getGuiTop() const { return (height - imageHeight) / 2; }
	int_t getSlotAt(int_t x, int_t y) const;
	void renderBg();

public:
	BrewingStandScreen(Minecraft &minecraft, std::shared_ptr<BrewingStandTileEntity> stand);

	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override { return false; }
	void removed() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
};
