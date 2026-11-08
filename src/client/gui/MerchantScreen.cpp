#include "client/gui/MerchantScreen.h"

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gamemode/GameMode.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "lwjgl/Keyboard.h"
#include "world/entity/npc/Merchant.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/MerchantMenu.h"
#include "world/inventory/Slot.h"
#include "world/stats/StatCollector.h"

void MerchantScreen::TradeButton::render(Minecraft &minecraft, int_t xm, int_t ym)
{
	if (!visible)
		return;
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/gui/trading.png"));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	bool hovered = xm >= x && ym >= y && xm < x + w && ym < y + h;
	int_t v = 0;
	int_t u = 176;
	if (!active)
		u += w * 2;
	else if (hovered)
		u += w;
	if (!forward)
		v += h;
	blit(x, y, u, v, w, h);
}

MerchantScreen::MerchantScreen(Minecraft &minecraft, std::shared_ptr<MerchantMenu> menu, Merchant &merchant, const jstring &name)
	: ContainerScreen(minecraft), menu(std::move(menu)), merchant(merchant)
{
	passEvents = true;
	title = !name.empty() ? name : StatCollector::translate(u"entity.Villager.name");
}

void MerchantScreen::init()
{
	buttons.clear();
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	nextButton = Util::make_shared<TradeButton>(1, xo + 120 + 27, yo + 24 - 1, true);
	prevButton = Util::make_shared<TradeButton>(2, xo + 36 - 19, yo + 24 - 1, false);
	buttons.push_back(nextButton);
	buttons.push_back(prevButton);
	nextButton->active = false;
	prevButton->active = false;
}

void MerchantScreen::tick()
{
	ContainerScreen::tick();
	if (minecraft.player == nullptr)
		return;
	MerchantRecipeList *offers = merchant.getOffers(*minecraft.player);
	if (offers != nullptr)
	{
		nextButton->active = currentRecipeIndex < static_cast<int_t>(offers->size()) - 1;
		prevButton->active = currentRecipeIndex > 0;
	}
}

void MerchantScreen::buttonClicked(Button &button)
{
	bool changed = false;
	if (&button == nextButton.get())
	{
		currentRecipeIndex++;
		changed = true;
	}
	else if (&button == prevButton.get())
	{
		currentRecipeIndex--;
		changed = true;
	}
	if (changed)
		menu->setSelectionHint(currentRecipeIndex);
}

int_t MerchantScreen::getSlotAt(int_t x, int_t y) const
{
	int_t relX = x - getGuiLeft();
	int_t relY = y - getGuiTop();
	for (size_t i = 0; i < menu->slots.size(); ++i)
	{
		const Slot &slot = *menu->slots[i];
		if (relX >= slot.x - 1 && relX < slot.x + 17 && relY >= slot.y - 1 && relY < slot.y + 17)
			return static_cast<int_t>(i);
	}
	return -1;
}

bool MerchantScreen::isHovering(int_t x, int_t y, int_t w, int_t h, int_t mx, int_t my) const
{
	mx -= getGuiLeft();
	my -= getGuiTop();
	return mx >= x - 1 && mx < x + w + 1 && my >= y - 1 && my < y + h + 1;
}

void MerchantScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground();
	if (minecraft.player == nullptr || menu == nullptr)
		return;
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();

	// renderBg
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(minecraft.textures.loadTexture(u"/gui/trading.png"));
	blit(xo, yo, 0, 0, imageWidth, imageHeight);
	MerchantRecipeList *offers = merchant.getOffers(*minecraft.player);
	if (offers != nullptr && !offers->empty() && (*offers)[currentRecipeIndex].isDeprecated())
	{
		glDisable(GL_LIGHTING);
		blit(xo + 83, yo + 21, 212, 0, 28, 21);
		blit(xo + 83, yo + 51, 212, 0, 28, 21);
	}

	for (auto &button : buttons)
		button->render(minecraft, xm, ym);

	int_t relX = xm - xo;
	int_t relY = ym - yo;
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
	glPushMatrix();
	glTranslatef(static_cast<float>(xo), static_cast<float>(yo), 0.0f);
	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();
	int_t hovered = getSlotAt(xm, ym);
	for (auto &slotPtr : menu->slots)
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
		const Slot &slot = *menu->slots[hovered];
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
		glTranslatef(0.0f, 0.0f, -32.0f);
	}
	glDisable(GL_RESCALE_NORMAL);
	Lighting::turnOff();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// renderLabels
	font.draw(title, imageWidth / 2 - font.width(title) / 2, 6, 0x404040);
	font.draw(StatCollector::translate(u"container.inventory"), 8, imageHeight - 96 + 2, 0x404040);

	if (carried == nullptr && hovered >= 0)
	{
		const ItemInstance *stack = menu->slots[hovered]->getItem();
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

	// the current offer: what to pay and what you get
	if (offers != nullptr && !offers->empty())
	{
		MerchantRecipe &recipe = (*offers)[currentRecipeIndex];
		ItemInstance buyA = recipe.getBuyA();
		const ItemInstance *buyBPtr = recipe.getBuyB();
		ItemInstance buyB = buyBPtr != nullptr ? *buyBPtr : ItemInstance();
		ItemInstance sell = recipe.getSellItem();
		glPushMatrix();
		glPushMatrix();
		glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
		Lighting::turnOn();
		glPopMatrix();
		glDisable(GL_LIGHTING);
		glEnable(GL_RESCALE_NORMAL);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glTranslatef(0.0f, 0.0f, 100.0f);
		itemRenderer.renderGuiItem(font, minecraft.textures, buyA, xo + 36, yo + 24);
		itemRenderer.renderGuiItemDecorations(font, minecraft.textures, buyA, xo + 36, yo + 24);
		if (!buyB.isEmpty())
		{
			itemRenderer.renderGuiItem(font, minecraft.textures, buyB, xo + 62, yo + 24);
			itemRenderer.renderGuiItemDecorations(font, minecraft.textures, buyB, xo + 62, yo + 24);
		}
		itemRenderer.renderGuiItem(font, minecraft.textures, sell, xo + 120, yo + 24);
		itemRenderer.renderGuiItemDecorations(font, minecraft.textures, sell, xo + 120, yo + 24);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		const ItemInstance *tip = nullptr;
		if (isHovering(36, 24, 16, 16, xm, ym))
			tip = &buyA;
		else if (!buyB.isEmpty() && isHovering(62, 24, 16, 16, xm, ym))
			tip = &buyB;
		else if (isHovering(120, 24, 16, 16, xm, ym))
			tip = &sell;
		if (tip != nullptr && carried == nullptr)
		{
			auto lines = getTooltipLines(*tip, &minecraft.options);
			if (!lines.empty())
				renderTooltip(lines, xm + 12, ym - 12);
		}
		glPopMatrix();
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		Lighting::turnOff();
		glDisable(GL_LIGHTING);
	}
	(void)a;
}

void MerchantScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE || eventKey == minecraft.options.keyInventory.key)
	{
		minecraft.player->closeContainer();
		minecraft.grabMouse();
		return;
	}
	Screen::keyPressed(eventCharacter, eventKey);
}

void MerchantScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	Screen::mouseClicked(x, y, buttonNum);
	if (minecraft.player == nullptr || menu == nullptr || (buttonNum != 0 && buttonNum != 1))
		return;
	for (auto &button : buttons)
		if (button->active && button->visible && x >= button->x && y >= button->y && x < button->x + 12 && y < button->y + 19)
			return;
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	bool outside = x < xo || y < yo || x >= xo + imageWidth || y >= yo + imageHeight;
	int_t slot = outside ? -999 : getSlotAt(x, y);
	if (slot == -1)
		return;
	bool shift = slot != -999 && (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) || lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT));
	minecraft.gameMode->clickContainer(menu->windowId, slot, buttonNum, shift, *minecraft.player);
}

void MerchantScreen::removed()
{
	if (minecraft.player != nullptr && menu != nullptr)
		minecraft.gameMode->closeContainer(menu->windowId, *minecraft.player);
	Screen::removed();
}
