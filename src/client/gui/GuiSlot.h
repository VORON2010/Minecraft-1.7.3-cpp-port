#pragma once

#include "client/gui/Button.h"
#include "client/gui/GuiComponent.h"

#include "java/Type.h"

class Minecraft;
class Tesselator;

class GuiSlot : public GuiComponent
{
private:
	int_t scrollUpButtonID = 0;
	int_t scrollDownButtonID = 0;
	float initialClickY = -2.0f;
	float scrollMultiplier = 1.0f;
	float amountScrolled = 0.0f;
	int_t selectedElement = -1;
	long_t lastClicked = 0;
	bool renderSelection = true;
	bool hasHeader = false;
	int_t headerPadding = 0;

protected:
	Minecraft &minecraft;
	int_t width;
	int_t height;
	int_t top;
	int_t bottom;
	int_t right;
	int_t left;
	int_t posZ;

public:
	GuiSlot(Minecraft &minecraft, int_t width, int_t height, int_t top, int_t bottom, int_t slotHeight);
	virtual ~GuiSlot() = default;

	void setRenderSelection(bool renderSelection);
	void setHeader(bool hasHeader, int_t headerPadding);
	int_t getSlotIndex(int_t x, int_t y);
	void registerScrollButtons(int_t scrollUpButtonID, int_t scrollDownButtonID);
	void actionPerformed(Button &button);
	void mouseScrolled(int_t scrollAmount);
	void drawScreen(int_t mouseX, int_t mouseY, float partialTick);

protected:
	virtual int_t getSize() = 0;
	virtual void elementClicked(int_t index, bool doubleClick) = 0;
	virtual bool isSelected(int_t index) = 0;
	virtual int_t getContentHeight();
	virtual void drawBackground() = 0;
	virtual void drawSlot(int_t index, int_t x, int_t y, int_t height, Tesselator &t) = 0;
	virtual void drawHeader(int_t x, int_t y, Tesselator &t);
	virtual void headerClicked(int_t x, int_t y);
	virtual void drawOverlay(int_t mouseX, int_t mouseY);

private:
	void bindAmountScrolled();
	void overlayBackground(int_t y0, int_t y1, int_t alphaTop, int_t alphaBottom);
};
