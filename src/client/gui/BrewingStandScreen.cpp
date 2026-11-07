#include "client/gui/BrewingStandScreen.h"

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gamemode/GameMode.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "lwjgl/Keyboard.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/Container.h"
#include "world/level/tile/entity/BrewingStandTileEntity.h"

BrewingStandScreen::BrewingStandScreen(Minecraft &minecraft, std::shared_ptr<BrewingStandTileEntity> stand)
	: ContainerScreen(minecraft), stand(std::move(stand)), containerMenu(minecraft.player->craftingInventory)
{
	passEvents = true;
}

int_t BrewingStandScreen::getSlotAt(int_t x, int_t y) const
{
	if (containerMenu == nullptr)
		return -1;
	int_t relX = x - getGuiLeft();
	int_t relY = y - getGuiTop();
	for (size_t i = 0; i < containerMenu->slots.size(); ++i)
	{
		const Slot &slot = *containerMenu->slots[i];
		if (relX >= slot.x - 1 && relX < slot.x + 17 && relY >= slot.y - 1 && relY < slot.y + 17)
			return static_cast<int_t>(i);
	}
	return -1;
}

void BrewingStandScreen::renderBg()
{
	int_t tex = minecraft.textures.loadTexture(u"/gui/alchemy.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(tex);
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	blit(xo, yo, 0, 0, imageWidth, imageHeight);
	int_t time = stand->getBrewTime();
	if (time > 0)
	{
		int_t arrow = static_cast<int_t>(28.0f * (1.0f - static_cast<float>(time) / 600.0f));
		if (arrow > 0)
			blit(xo + 97, yo + 16, 176, 0, 9, arrow);
		static const int_t bubbles[7] = {29, 24, 20, 16, 11, 6, 0};
		int_t b = bubbles[time / 2 % 7];
		if (b > 0)
			blit(xo + 65, yo + 14 + 29 - b, 185, 29 - b, 12, b);
	}
}

void BrewingStandScreen::render(int_t xm, int_t ym, float a)
{
	(void)a;
	renderBackground();
	if (minecraft.player == nullptr || stand == nullptr || containerMenu == nullptr)
		return;
	renderBg();

	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	int_t relX = xm - xo;
	int_t relY = ym - yo;
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);

	glPushMatrix();
	glTranslatef(static_cast<float>(xo), static_cast<float>(yo), 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();

	int_t hovered = getSlotAt(xm, ym);
	for (auto &slotPtr : containerMenu->slots)
	{
		ItemInstance *stack = slotPtr->getItem();
		if (stack != nullptr && !stack->isEmpty())
		{
			itemRenderer.renderGuiItem(font, minecraft.textures, *stack, slotPtr->x, slotPtr->y);
			itemRenderer.renderGuiItemDecorations(font, minecraft.textures, *stack, slotPtr->x, slotPtr->y);
		}
	}
	if (hovered >= 0)
	{
		const Slot &slot = *containerMenu->slots[hovered];
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		fillGradient(slot.x, slot.y, slot.x + 16, slot.y + 16, 0x80FFFFFF, 0x80FFFFFF);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}

	ItemInstance *carried = minecraft.player->inventory.getCarried();
	if (carried != nullptr && !carried->isEmpty())
	{
		glTranslatef(0.0f, 0.0f, 32.0f);
		itemRenderer.renderGuiItem(font, minecraft.textures, *carried, relX - 8, relY - 8);
		itemRenderer.renderGuiItemDecorations(font, minecraft.textures, *carried, relX - 8, relY - 8);
	}

	glDisable(GL_RESCALE_NORMAL);
	Lighting::turnOff();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	font.draw(u"Brewing Stand", 56, 6, 0x404040);
	font.draw(u"Inventory", 8, imageHeight - 96 + 2, 0x404040);
	if (carried == nullptr && hovered >= 0)
	{
		const ItemInstance *stack = containerMenu->slots[hovered]->getItem();
		if (stack != nullptr && !stack->isEmpty())
		{
			auto lines = getTooltipLines(*stack, &minecraft.options);
			if (!lines.empty())
				renderTooltip(lines, relX + 12, relY - 12);
		}
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

void BrewingStandScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE || eventKey == minecraft.options.keyInventory.key)
	{
		minecraft.player->closeContainer();
		minecraft.grabMouse();
		return;
	}
	Screen::keyPressed(eventCharacter, eventKey);
}

void BrewingStandScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	if (minecraft.player == nullptr || containerMenu == nullptr || (buttonNum != 0 && buttonNum != 1))
		return;
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	bool outside = x < xo || y < yo || x >= xo + imageWidth || y >= yo + imageHeight;
	int_t slot = outside ? -999 : getSlotAt(x, y);
	if (slot == -1)
		return;
	bool shift = slot != -999 && (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) || lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT));
	minecraft.gameMode->clickContainer(containerMenu->windowId, slot, buttonNum, shift, *minecraft.player);
}

void BrewingStandScreen::removed()
{
	if (minecraft.player != nullptr && containerMenu != nullptr)
		minecraft.gameMode->closeContainer(containerMenu->windowId, *minecraft.player);
	Screen::removed();
}
