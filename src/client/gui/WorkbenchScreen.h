#pragma once

#include "client/gui/InventoryScreen.h"

class Level;

class WorkbenchScreen : public InventoryScreen
{
protected:
	jstring getBackgroundTexture() const override;
	int_t getCraftingGridLeft() const override;
	int_t getCraftingGridTop() const override;
	int_t getResultSlotX() const override;
	int_t getResultSlotY() const override;
	int_t getTitleX() const override;
	int_t getTitleY() const override;
	jstring getTitleText() const override;
	void renderLabels() override;
	bool shouldRenderPlayerModel() const override;

public:
	WorkbenchScreen(Minecraft &minecraft, Level &level, int_t x, int_t y, int_t z);

};
