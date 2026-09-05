#include "client/gui/WorkbenchScreen.h"

WorkbenchScreen::WorkbenchScreen(Minecraft &minecraft, Level &, int_t, int_t, int_t)
	: InventoryScreen(minecraft, 3, 3)
{
}

jstring WorkbenchScreen::getBackgroundTexture() const
{
	return u"/gui/crafting.png";
}

int_t WorkbenchScreen::getCraftingGridLeft() const
{
	return 30;
}

int_t WorkbenchScreen::getCraftingGridTop() const
{
	return 17;
}

int_t WorkbenchScreen::getResultSlotX() const
{
	return 124;
}

int_t WorkbenchScreen::getResultSlotY() const
{
	return 35;
}

int_t WorkbenchScreen::getTitleX() const
{
	return 28;
}

int_t WorkbenchScreen::getTitleY() const
{
	return 6;
}

jstring WorkbenchScreen::getTitleText() const
{
	return u"Crafting";
}


void WorkbenchScreen::renderLabels()
{
	font.draw(u"Crafting", 28, 6, 0x404040);
	font.draw(u"Inventory", 8, 166 - 96 + 2, 0x404040);
}

bool WorkbenchScreen::shouldRenderPlayerModel() const
{
	return false;
}
